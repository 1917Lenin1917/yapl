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
    int current_col_pos = 0;
    int paren_depth = 0;
    int brace_depth = 0;
    int sq_br_depth = 0;
    bool inside_import = false;
public:
    explicit Lexer(const std::string_view text)
        : m_text(text), m_pos(-1) {}

    std::vector<Token> make_tokens();

private:
    void check_insert_semicolon(std::vector<Token>& tokens);
    Token make_number();
    Token make_string();
    Token make_format_string();
    Token make_identifier_or_keyword();

};

bool is_numeric(char c);
bool is_letter(char c);

}

