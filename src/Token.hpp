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
    PLUS = 0,
    MINUS,
    MUL,
    DIV,
    LPAREN,
    RPAREN,
    SQOUTE,
    DQOUTE,
    IDENTIFIER,
    INTEGER,
    FLOAT,
    STRING,
};

struct Token
{
    TOKEN_TYPE type = TOKEN_TYPE::DEFAULT;
    char* value = nullptr;

};

std::string ttype_to_string(TOKEN_TYPE tt);

std::string print_token(const Token& token);
}