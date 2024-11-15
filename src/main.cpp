#include <chrono>
#include <format>
#include <iostream>
#include <fstream>

#include "Lexer.hpp"
#include "Parser.hpp"

using namespace yapl;

void run()
{
  Interpreter intp;
  Visitor v{intp};

  while (true)
  {
    std::string input;
    std::getline(std::cin, input);

    auto tokens = Lexer{input}.make_tokens();
    auto ast = Parser{tokens}.parse_root();

    ast->visit(v);
  }
}


int main()
{
  // run();
  // return 0;
  // std::ifstream t("C:\\_projects\\yapl\\test.txt");
  // std::string text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
  // std::string text {"fn penis(): int { let a = 5; }; fn bebra(a: int, b: int): int { a + b; };"};
  std::string text{"bebra();"};

  Lexer lexer {text};
  auto ltime1 = std::chrono::system_clock::now();
  auto tokens = lexer.make_tokens();
  auto ltime2 = std::chrono::system_clock::now();
  std::cout << "Tokenizing " << tokens.size() << " tokens took: " << std::chrono::duration_cast<std::chrono::milliseconds>(ltime2-ltime1) << "\n";


  Parser parser {tokens};
  auto rtime1 = std::chrono::system_clock::now();
  auto ast = parser.parse_root();
  auto rtime2 = std::chrono::system_clock::now();
  std::cout << "Parsing took: " << std::chrono::duration_cast<std::chrono::milliseconds>(rtime2-rtime1) << "\n";
  std::cout << std::endl;
  std::cout << ast->print() << "\n";

  Interpreter intp;
  Visitor v{intp};

  ast->visit(v);

}
