#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <sstream>

#include "yapl/Lexer.hpp"
#include "yapl/Parser.hpp"
#include "yapl/Interpreter.hpp"
#include "yapl/Visitor.hpp"

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
//    run();
  std::ifstream t(argv[1]);
  // std::ifstream t(R"(C:\_projects\yapl\test.yapl)");
  std::string text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  Lexer lexer {text};
  auto ltime1 = std::chrono::system_clock::now();
  auto tokens = lexer.make_tokens();
  auto ltime2 = std::chrono::system_clock::now();
  std::cout << "Tokenizing " << tokens.size() << " tokens took: " << std::chrono::duration_cast<std::chrono::microseconds>(ltime2-ltime1) << "\n";


  auto filename = std::filesystem::path(R"(C:\_projects\yapl\test.yapl)").filename().string();
  auto lines = get_lines_from_text(text);
  Parser parser {tokens, filename, lines};
  auto rtime1 = std::chrono::system_clock::now();
  auto ast = parser.parse_root();
  auto rtime2 = std::chrono::system_clock::now();
  std::cout << "Parsing took: " << std::chrono::duration_cast<std::chrono::microseconds>(rtime2-rtime1) << "\n";
  std::cout << std::endl;
  std::cout << ast->print(0) << "\n";

  Interpreter intp;
  intp.base_path = std::filesystem::path(R"(C:\_projects\yapl\examples\src\)");
  Visitor v{intp};

  auto vtime1 = std::chrono::system_clock::now();
  ast->visit(v);
  auto vtime2 = std::chrono::system_clock::now();
  std::cout << "Interpreting took: " << std::chrono::duration_cast<std::chrono::milliseconds>(vtime2-vtime1) << "\n";
  std::cout << std::endl;
  return 0;
}
// TODO: change variable lookup, so it doesn't go beyond current function scope + global. Additionally think about closure implementation.