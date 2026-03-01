#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <string>
#include <clocale>

#include <yapl/values/ArrayValue.hpp>
#include <yapl/values/FunctionValue.hpp>
#include <yapl/values/TypeObjectValue.hpp>

#include "yapl/ByteCodeVM.hpp"
#include "yapl/Lexer.hpp"
#include "yapl/Parser.hpp"
#include "yapl/values/UndefinedValue.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/CodeObjectValue.hpp"
#include "yapl/Hasher.hpp"
#include "yapl/Resolver.hpp"
#include "yapl/Utils.hpp"
#include "yapl/values/ModuleValue.hpp"

using namespace yapl;

void initialize()
{
  init_builtin_function_tp();
  init_int_tp();
  init_float_type();
  init_bool_tp();
  init_str_tp();
  init_tp_tp();
  init_dict_tp();
  init_function_tp();
  init_size_iterator_type();
  init_undefined_tp();
  init_code_object_type_object();
  init_array_tp();
  init_module_type_object();
}

void recursively_print_code_objects(const CodeObject &code_object)
{
  print_code_object(code_object);
  std::cout << "\n\n";
  for (const auto &constant : code_object.constants)
  {
    if (const auto code_object_value = dynamic_cast<CodeObjectValue *>(constant.get()))
    {
      recursively_print_code_objects(*code_object_value->code_object);
    }
  }
}

struct ParsedUnit
{
  std::string text;
  std::vector<Token> tokens;
  std::vector<std::string> lines;
  std::unique_ptr<BaseASTNode> ast;
  ResolutionResult resolver_output;
};

ParsedUnit parse_and_resolve_file(
  const std::filesystem::path &full_path,
  const std::filesystem::path &filename
)
{
  std::ifstream source_file(full_path);
  if (!source_file.is_open())
  {
    throw std::runtime_error("Failed to open source file: " + full_path.string());
  }

  std::string text(
    (std::istreambuf_iterator<char>(source_file)),
    std::istreambuf_iterator<char>()
  );

  Lexer lexer{text};
  auto tokens = lexer.make_tokens();

  auto lines = get_lines_from_text(text);
  Parser parser{tokens, filename.string(), lines};
  auto ast = parser.parse_root();
  auto *root_ast_node = static_cast<RootASTNode *>(ast.get());

  Resolver resolver;
  auto resolver_output = resolver.Resolve(*root_ast_node);

  return ParsedUnit{
    std::move(text),
    std::move(tokens),
    std::move(lines),
    std::move(ast),
    std::move(resolver_output)
  };
}

CodeObject build_code_object_from_ast(RootASTNode &root_ast_node)
{
  ByteCodeVisitor visitor;
  return visitor.visit_RootASTNode(root_ast_node);
}

int run_diagnostics_mode(const std::filesystem::path &full_path)
{
  const auto filename = full_path.filename();

  try
  {
    auto parsed_unit = parse_and_resolve_file(full_path, filename);
    serialize_json(parsed_unit.resolver_output, std::cout);
    std::cout << '\n';
    return 0;
  }
  catch (const std::exception &error)
  {
    std::cerr << error.what() << '\n';
    return 1;
  }
}

int run_vm_mode(const std::filesystem::path &full_path)
{
  const auto path = full_path.parent_path();
  const auto filename = full_path.filename();

  CodeObject code_object;
  bool is_valid = is_valid_cache(filename);
  // bool is_valid = false;

  if (!is_valid)
  {
    std::cout << "Cache is invalid, parsing file from source.\n";

    auto started_at = std::chrono::system_clock::now();

    auto parsed_unit = parse_and_resolve_file(full_path, filename);
    auto *root_ast_node = static_cast<RootASTNode *>(parsed_unit.ast.get());
    code_object = build_code_object_from_ast(*root_ast_node);

    auto finished_at = std::chrono::system_clock::now();
    std::cout << "Parsing file from source took: "
              << std::chrono::duration_cast<std::chrono::microseconds>(finished_at - started_at)
              << "\n";

    auto code_object_bytes = code_object.Serialize();
    auto bytes = serialize_module(code_object_bytes, path / filename);
    auto cache_path = path / ".cache";
    std::filesystem::create_directories(cache_path);

    auto cache_file_path = cache_path / filename;
    cache_file_path.replace_extension("yaplcache");

    std::ofstream cache_file(cache_file_path, std::ios::binary);
    cache_file.write(
      reinterpret_cast<const char *>(bytes.data()),
      static_cast<std::streamsize>(bytes.size())
    );
  }
  else
  {
    std::cout << "Loading file from cache...\n";
    auto started_at = std::chrono::system_clock::now();

    auto cache_file_path = path / ".cache" / filename;
    cache_file_path.replace_extension("yaplcache");

    std::ifstream cache_file(cache_file_path, std::ios::binary);
    cache_file.seekg(33, std::ios::beg);

    cache_file.seekg(0, std::ios::end);
    const std::streamsize size = cache_file.tellg();
    cache_file.seekg(33, std::ios::beg);

    std::vector<std::byte> bytes(size - 33);
    cache_file.read(reinterpret_cast<char *>(bytes.data()), size);

    std::size_t offset = 0;
    code_object = CodeObject::Deserialize(bytes, offset);

    auto finished_at = std::chrono::system_clock::now();
    std::cout << "Loading from cache took: "
              << std::chrono::duration_cast<std::chrono::microseconds>(finished_at - started_at)
              << "\n";
  }

  try
  {
    auto started_at = std::chrono::system_clock::now();
    ByteCodeVM vm{code_object};
    vm.base_path = path;
    vm.Run();
    auto finished_at = std::chrono::system_clock::now();
    std::cout << "Virtual Byting took: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(finished_at - started_at)
              << "\n";
    std::cout << std::endl;
    return 0;
  }
  catch (const std::runtime_error &error)
  {
    std::cerr << error.what() << std::endl;
    return 1;
  }
}

int main(int argc, char **argv)
{
  std::setlocale(LC_ALL, "ru_RU.utf-8");

  if (argc < 2)
  {
    std::cerr << "Usage:\n";
    std::cerr << "  yapl <file>\n";
    std::cerr << "  yapl --diagnostics <file>\n";
    return 1;
  }

  initialize();

  const std::string first_argument = argv[1];

  if (first_argument == "--diagnostics")
  {
    if (argc < 3)
    {
      std::cerr << "Missing file path for diagnostics mode\n";
      return 1;
    }

    const auto full_path = std::filesystem::path(argv[2]);
    return run_diagnostics_mode(full_path);
  }

  const auto full_path = std::filesystem::path(argv[1]);
  return run_vm_mode(full_path);
}
