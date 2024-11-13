//
// Created by lenin on 13.11.2024.
//

#include "Token.hpp"

namespace yapl {

std::string ttype_to_string(TOKEN_TYPE tt)
{
    switch (tt)
    {
        case TOKEN_TYPE::PLUS:       { return "PLUS"; }
        case TOKEN_TYPE::MINUS:      { return "MINUS"; }
        case TOKEN_TYPE::MUL:        { return "MULTIPLY"; }
        case TOKEN_TYPE::DIV:        { return "DIVIDE"; }
        case TOKEN_TYPE::LPAREN:     { return "LPAREN"; }
        case TOKEN_TYPE::RPAREN:     { return "RPAREN"; }
        case TOKEN_TYPE::IDENTIFIER: { return "IDENTIFIER"; }
        case TOKEN_TYPE::INTEGER:    { return "INTEGER"; }
        case TOKEN_TYPE::FLOAT:      { return "FLOAT"; }
        default: { assert(false); }
    }
    return "";
}

std::string print_token(const Token &token)
{
    if (token.value != nullptr)
        return std::format("{}:{}", ttype_to_string(token.type), token.value);
    return std::format("{}", ttype_to_string(token.type));
}
}
