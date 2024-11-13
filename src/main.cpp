#include <format>
#include <iostream>

#include "Lexer.hpp"

int main()
{
    const char* text = "fn bar() { const foo = 23; return `{foo}`; }";
    yapl::Lexer lexer {text};
    auto tokens = lexer.make_tokens();

    for (const auto& token : tokens)
    {
        std::cout << yapl::print_token(token) << " ";
    }

}
