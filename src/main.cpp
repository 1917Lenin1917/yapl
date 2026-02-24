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
#include "yapl/values/UndefinedValue.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/CodeObjectValue.hpp"

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

int main(int argc, char** argv)
{
  std::setlocale(LC_ALL, "ru_RU.utf-8");
  std::ifstream t(argv[1]);
  std::string text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  Lexer lexer {text};
  auto tokens = lexer.make_tokens();

  auto filename = std::filesystem::path(argv[1]).filename().string();
  auto lines = get_lines_from_text(text);
  Parser parser {tokens, filename, lines};
  auto ast = parser.parse_root();
  auto ast_as_root = static_cast<RootASTNode*>(ast.get());

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

  ByteCodeVisitor v;

  auto vmtime1 = std::chrono::system_clock::now();
  auto obj = v.visit_RootASTNode(*ast_as_root);
  recursively_print_code_objects(obj);
  try
  {
    ByteCodeVM vm { obj };
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