#include <format>
#include <iostream>

#include "Lexer.hpp"
#include "Parser.hpp"

using namespace yapl;

int main()
{
    const char* text = "const penis = 420 * (foo + 27 * 222) / 123.0 - bar";
    Lexer lexer {text};
    auto tokens = lexer.make_tokens();

    Parser parser {tokens};
    auto ast = parser.parse_var_decl();
    std::cout << ast->print();
}
