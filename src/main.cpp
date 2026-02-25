#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>
#include <yapl/values/ArrayValue.hpp>
#include <yapl/values/FunctionValue.hpp>
#include <yapl/values/TypeObjectValue.hpp>

#include "yapl/ByteCodeVM.hpp"
#include "yapl/Lexer.hpp"
#include "yapl/Parser.hpp"
#include "yapl/Serialization.hpp"
#include "yapl/values/UndefinedValue.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/CodeObjectValue.hpp"
#include "yapl/Hasher.hpp"

using namespace yapl;

std::vector<std::string> get_lines_from_text(const std::string& text)
{
    std::vector<std::string> res;
    std::stringstream ss { text };
    while (!ss.eof())
    {
        std::string line;
        std::getline(ss, line);
        res.push_back(line);
    }
    return res;
}

void recursively_print_code_objects(const CodeObject& co)
{
  print_code_object(co);
  std::cout << "\n\n";
  for (const auto& constant : co.constants)
  {
    if (const auto cov = dynamic_cast<CodeObjectValue*>(constant.get()))
    {
      recursively_print_code_objects(*cov->code_object);
    }
  }
}

constexpr std::uint8_t MODULE_VERSION = 1;

std::vector<std::byte> serialize_module(const std::vector<std::byte>& co_bytes, const std::filesystem::path& module_path)
{
  std::vector<std::byte> result;
  appendByte(result, static_cast<std::byte>(MODULE_VERSION));
  const auto hash = md5HexFromFile(module_path);
  appendStringBytes(result, hash);
  result.insert(result.end(), co_bytes.begin(), co_bytes.end());

  return result;
}

ByteCodeVisitor initialize()
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

  return ByteCodeVisitor{};
}

bool is_valid_cache(const std::filesystem::path& module_path)
{
  auto cache_path = module_path.parent_path() / ".cache" / module_path.filename().replace_extension("yaplcache");
  auto new_md5 = md5HexFromFile(module_path);
  std::ifstream t(cache_path, std::ios::binary);
  if (!t.is_open())
    return false;

  std::uint8_t version = 0;
  t.read(reinterpret_cast<std::istream::char_type *>(&version), 1);

  assert(("Module version is wrong!", version == MODULE_VERSION));

  std::string old_md5;
  old_md5.resize(32);
  t.read(old_md5.data(), 32);

  t.close();

  return old_md5 == new_md5;
}

int main(int argc, char** argv)
{
  std::setlocale(LC_ALL, "ru_RU.utf-8");

  const auto full_path = std::filesystem::path(argv[1]);
  auto path = full_path.parent_path();
  auto filename = full_path.filename();

  auto v = initialize();

  bool is_valid = is_valid_cache(full_path);

  CodeObject co;
  if (!is_valid)
  {
    std::cout << "Cache is invalid, parsing file from source.\n";
    auto vmtime1 = std::chrono::system_clock::now();
    std::ifstream t(argv[1]);
    std::string text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    Lexer lexer {text};
    auto tokens = lexer.make_tokens();

    auto lines = get_lines_from_text(text);
    Parser parser {tokens, filename.string(), lines};
    auto ast = parser.parse_root();
    auto ast_as_root = static_cast<RootASTNode*>(ast.get());

    co = v.visit_RootASTNode(*ast_as_root);
    // recursively_print_code_objects(co);

    auto co_bytes = co.Serialize();
    auto bytes = serialize_module(co_bytes, path / filename);
    auto cache_path = path / ".cache";
    std::filesystem::create_directories(cache_path);

    std::ofstream cache(cache_path / filename.replace_extension("yaplcache"), std::ios::binary);
    cache.write(
      reinterpret_cast<const char*>(bytes.data()),
      static_cast<std::streamsize>(bytes.size())
    );

    auto vmtime2 = std::chrono::system_clock::now();
    std::cout << "Parsing file from source took: " << std::chrono::duration_cast<std::chrono::microseconds>(vmtime2-vmtime1) << "\n";

  }
  else
  {
    std::cout << "Loading file from cache...\n";
    auto vmtime1 = std::chrono::system_clock::now();

    auto cache_path = path / ".cache" / filename.replace_extension("yaplcache");
    std::ifstream fin(cache_path, std::ios::binary);
    fin.seekg(33, std::ios::beg);

    fin.seekg(0, std::ios::end);
    const std::streamsize size = fin.tellg();
    fin.seekg(33, std::ios::beg);

    std::vector<std::byte> bytes(size - 33);
    fin.read(reinterpret_cast<char*>(bytes.data()), size);

    std::size_t offset = 0;
    co = CodeObject::Deserialize(bytes, offset);

    auto vmtime2 = std::chrono::system_clock::now();
    std::cout << "Loading from cache took: " << std::chrono::duration_cast<std::chrono::microseconds>(vmtime2-vmtime1) << "\n";
  }

  try
  {
    auto vmtime1 = std::chrono::system_clock::now();
    ByteCodeVM vm { co };
    vm.Run();
    auto vmtime2 = std::chrono::system_clock::now();
    std::cout << "Virtual Byting took: " << std::chrono::duration_cast<std::chrono::milliseconds>(vmtime2-vmtime1) << "\n";
    std::cout << std::endl;
  }
  catch (const std::runtime_error& e)
  {
      std::cerr << e.what() << std::endl;
  }
  return 0;
}