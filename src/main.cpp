#include <format>
#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"

using namespace yapl;


int main()
{
    const char* text = "(1 + (8 * 2))";
    Lexer lexer {text};
    auto tokens = lexer.make_tokens();

    Parser parser {tokens};
    auto ast = parser.parse_expr();
    std::cout << ast->print() << "\n" << ast->evaluate();
}
