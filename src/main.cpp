#include <format>
#include <iostream>

#include "Lexer.hpp"

int main()
{
    const char* text = "(111111 + 22) * 3 / 69.69";
    yapl::Lexer lexer {text};
    auto tokens = lexer.make_tokens();

    for (const auto& token : tokens)
    {
        std::cout << yapl::print_token(token) << " ";
    }

}
