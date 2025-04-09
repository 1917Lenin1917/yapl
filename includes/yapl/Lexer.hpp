//
// Created by lenin on 13.11.2024.
//

#pragma once
#include <cassert>
#include <string_view>
#include <vector>
#include <cstring>

#include "Token.hpp"

namespace yapl {


class Lexer
{
private:
    std::string_view m_text;
    size_t m_pos;
    int current_line = 1;
    int current_col_pos = 1;
public:
    explicit Lexer(const std::string_view text)
        : m_text(text), m_pos(-1) {}

    std::vector<Token> make_tokens();

private:
    Token make_number();
    Token make_string();
    Token make_format_string();
    Token make_identifier_or_keyword();

};

bool is_numeric(char c);
bool is_letter(char c);

}

