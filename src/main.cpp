#include <chrono>
#include <fstream>
#include <iostream>
#include <filesystem>

#include "yapl/Lexer.hpp"
#include "yapl/Parser.hpp"
#include "yapl/Interpreter.hpp"
#include "yapl/Visitor.hpp"

using namespace yapl;

void run()
{
//  Interpreter intp;
//  Visitor v{intp};
//
//  while (true)
//  {
//    std::string input;
//    std::getline(std::cin, input);
//
//    auto tokens = Lexer{input}.make_tokens();
//    auto ast = Parser{tokens}.parse_root();
//
//    ast->visit(v);
//  }
}

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


int main(int argc, char** argv)
{
  std::ifstream t(argv[1]);
  std::string text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  Lexer lexer {text};
  auto ltime1 = std::chrono::system_clock::now();
  auto tokens = lexer.make_tokens();
  auto ltime2 = std::chrono::system_clock::now();
  std::cout << "Tokenizing " << tokens.size() << " tokens took: " << std::chrono::duration_cast<std::chrono::milliseconds>(ltime2-ltime1) << "\n";


  auto filename = std::filesystem::path(argv[1]).filename().string();
  auto lines = get_lines_from_text(text);
  Parser parser {tokens, filename, lines};
  auto rtime1 = std::chrono::system_clock::now();
  auto ast = parser.parse_root();
  auto rtime2 = std::chrono::system_clock::now();
  std::cout << "Parsing took: " << std::chrono::duration_cast<std::chrono::milliseconds>(rtime2-rtime1) << "\n";
  std::cout << std::endl;
  std::cout << ast->print(0) << "\n";
//
//  Interpreter intp;
//  Visitor v{intp};
//
//  ast->visit(v);
//  std::cout << "Interpreting took: " << std::chrono::duration_cast<std::chrono::milliseconds>(rtime2-rtime1) << "\n";
//  std::cout << std::endl;
  return 0;
}
