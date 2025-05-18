//
// Created by lenin on 13.11.2024.
//

#pragma once
#include <string>
#include <cassert>
#include <format>
#include <iostream>
#include <cstring>

namespace yapl {

enum class TOKEN_TYPE
{
    DEFAULT = -1,
    // LITERALS
    INTEGER,
    FLOAT,
    STRING,
    FSTRING,
    BOOL,

    // OPERATORS
    PLUS,   // +
    PLUSEQ, // +=
    MINUS,  // -
    MINUSEQ,// -=
    TIMES,  // *
    TIMESEQ,// *=
    MOD,    // %
    MODEQ,  // %=
    SLASH,  // /
    SLASHEQ,// /=
    PERIOD, // .
    NOT,    // !
    ASSIGN, // =
    EQ,     // ==
    NEQ,    // !=
    LT,     // <
    LQ,     // <=
    GT,     // >
    GQ,     // >=
    ARROW,  // =>
    OR,     // or
    AND,    // and

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
    CLASS,

    // UNIQUE
    IDENTIFIER,
    TT_EOF,
};

struct Token
{
    char* value = nullptr;
    TOKEN_TYPE type = TOKEN_TYPE::DEFAULT;

    int line = -1;
    int col_start = -1;
    int col_end = -1;

    Token(TOKEN_TYPE t, char* val = nullptr, int l = -1, int cstart = -1, int cend = -1)
            : type(t), value(val), line(l), col_start(cstart), col_end(cend) {}
};

std::string ttype_to_string(TOKEN_TYPE tt);

std::string print_token(const Token& token);
}