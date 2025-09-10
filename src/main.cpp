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
#include "yapl/Interpreter.hpp"
#include "yapl/Visitor.hpp"
#include "yapl/values/UndefinedValue.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/BooleanValue.hpp"

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

void run()
{
  Interpreter intp;
  Visitor v{intp};
  // FIXME: дефенишн функции должен дип-копироваться отдельно в интерпретатор, т.к. при создании нового парсера, он просто пропадает, что в корне неверно!!!!!!

  while (true)
  {
    std::cout << ">> ";
    std::string input;
    std::getline(std::cin, input);

    auto tokens = Lexer{input}.make_tokens();
    auto ast = Parser{tokens, "<repl>", get_lines_from_text(input)}.parse_root();

    auto value = ast->visit(v);
    if (value)
        std::cout << value->print() << "\n";
  }
}

int main(int argc, char** argv)
{
  // auto VM = ByteCodeVM({
  //   LOAD_CONST, static_cast<OpCode>(2),
  //   LOAD_NAME, static_cast<OpCode>(1),
  //   BINARY_OP, static_cast<OpCode>(LT),
  //   JMP_IF_FALSE, static_cast<OpCode>(18),
  //
  //   LOAD_CONST, static_cast<OpCode>(0),
  //   LOAD_CONST, static_cast<OpCode>(1),
  //   BINARY_OP, static_cast<OpCode>(ADD),
  //   STORE_NAME, static_cast<OpCode>(0),
  //
  //   LOAD_NAME, static_cast<OpCode>(1),
  //   LOAD_CONST, static_cast<OpCode>(3),
  //   BINARY_OP, static_cast<OpCode>(ADD),
  //   STORE_NAME, static_cast<OpCode>(1),
  //
  //   JMP, static_cast<OpCode>(-26),
  //
  //   HALT,
  // });
  // auto rtime1 = std::chrono::system_clock::now();
  // VM.Run();
  // auto rtime2 = std::chrono::system_clock::now();
  // std::cout << "VM bytecode took: " << std::chrono::duration_cast<std::chrono::milliseconds>(rtime2-rtime1) << "\n";
  //
  std::ifstream t(argv[1]);
  std::string text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  Lexer lexer {text};
  auto tokens = lexer.make_tokens();

  auto filename = std::filesystem::path(argv[1]).filename().string();
  auto lines = get_lines_from_text(text);
  Parser parser {tokens, filename, lines};
  auto ast = parser.parse_root();
  auto ast_as_root = static_cast<RootASTNode*>(ast.get());


  init_int_tp();
  init_float_type();
  init_array_tp();
  init_bool_tp();
  init_str_tp();
  init_tp_tp();
  init_dict_tp();
  init_function_tp();
  init_size_iterator_type();
  init_undefined_tp();

  ByteCodeVisitor v;

  auto vmtime1 = std::chrono::system_clock::now();
  auto obj = v.visit_RootASTNode(*ast_as_root);
  ByteCodeVM vm { obj };
  vm.Run();
  auto vmtime2 = std::chrono::system_clock::now();
  std::cout << "Virtual Byting took: " << std::chrono::duration_cast<std::chrono::milliseconds>(vmtime2-vmtime1) << "\n";
  std::cout << std::endl;

  Interpreter intp;
  intp.base_path = std::filesystem::path(R"(C:\_projects\yapl\examples\src\)");
  Visitor vi{intp};

  auto vtime1 = std::chrono::system_clock::now();
  ast->visit(vi);
  auto vtime2 = std::chrono::system_clock::now();
  std::cout << "Interpreting took: " << std::chrono::duration_cast<std::chrono::milliseconds>(vtime2-vtime1) << "\n";
  std::cout << std::endl;
  return 0;

// //    run();
//     std::cout << argv[1];
//   std::ifstream t(argv[1]);
//   // std::ifstream t(R"(C:\_projects\yapl\test.yapl)");
//   std::string text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
//   Lexer lexer {text};
//   auto ltime1 = std::chrono::system_clock::now();
//   auto tokens = lexer.make_tokens();
//   auto ltime2 = std::chrono::system_clock::now();
//   std::cout << "Tokenizing " << tokens.size() << " tokens took: " << std::chrono::duration_cast<std::chrono::microseconds>(ltime2-ltime1) << "\n";
//
//
//   auto filename = std::filesystem::path(R"(C:\_projects\yapl\test.yapl)").filename().string();
//   auto lines = get_lines_from_text(text);
//   Parser parser {tokens, filename, lines};
//   auto rtime1 = std::chrono::system_clock::now();
//   auto ast = parser.parse_root();
//   auto rtime2 = std::chrono::system_clock::now();
//   std::cout << "Parsing took: " << std::chrono::duration_cast<std::chrono::microseconds>(rtime2-rtime1) << "\n";
//   std::cout << std::endl;
//   std::cout << ast->print(0) << "\n";
//
//   Interpreter intp;
//   intp.base_path = std::filesystem::path(R"(C:\_projects\yapl\examples\src\)");
//   Visitor v{intp};
//
//   auto vtime1 = std::chrono::system_clock::now();
//   ast->visit(v);
//   auto vtime2 = std::chrono::system_clock::now();
//   std::cout << "Interpreting took: " << std::chrono::duration_cast<std::chrono::milliseconds>(vtime2-vtime1) << "\n";
//   std::cout << std::endl;
//   return 0;
}
// TODO: change variable lookup, so it doesn't go beyond current function scope + global. Additionally think about closure implementation.