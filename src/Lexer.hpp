//
// Created by lenin on 13.11.2024.
//

#pragma once
#include <cassert>
#include <string_view>
#include <vector>

#include "Token.hpp"

namespace yapl {


class Lexer
{
private:
    std::string_view m_text;
    size_t m_pos;
public:
    explicit Lexer(const std::string_view text)
        : m_text(text), m_pos(-1) {}

    std::vector<Token> make_tokens();

private:
    Token make_number();

};

}

