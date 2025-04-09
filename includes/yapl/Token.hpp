//
// Created by lenin on 13.11.2024.
//

#pragma once
#include <string>
#include <cassert>
#include <format>

namespace yapl {

enum class TOKEN_TYPE
{
    DEFAULT = -1,
    // LITERALS
    INTEGER,
    FLOAT,
    STRING,
    FSTRING, // ????
    BOOL,

    // OPERATORS
    PLUS,
    MINUS,
    TIMES,
    SLASH,
    PERIOD,
    NOT,
    ASSIGN,
    EQ,
    LT,
    GT,
    LQ,
    GQ,
    ARROW, // =>

    // SEPARATORS
    LPAREN,
    RPAREN,
    LBRACK,
    RBRACK,
    LSQBRACK,
    RSQBRACK,
    SEMICOLON,
    COLON,
    COMMA,

    // KEYWORDS
    IF,
    ELSE,
    FOR,
    FN,
    VAR,
    LET,
    CONST,
    RETURN,
    WHILE,

    // UNIQUE
    IDENTIFIER,
    TT_EOF,
};

struct Token
{
    TOKEN_TYPE type = TOKEN_TYPE::DEFAULT;
    char* value = nullptr;

    int line = -1;
    int col_start = -1;
    int col_end = -1;
};

std::string ttype_to_string(TOKEN_TYPE tt);

std::string print_token(const Token& token);
}