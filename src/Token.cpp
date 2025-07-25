//
// Created by lenin on 13.11.2024.
//

#include "yapl/Token.hpp"

namespace yapl {

std::string ttype_to_string(TOKEN_TYPE tt)
{
switch (tt)
{
    // LITERALS
    case TOKEN_TYPE::INTEGER:   { return "INTEGER"; }
    case TOKEN_TYPE::FLOAT:     { return "FLOAT"; }
    case TOKEN_TYPE::STRING:    { return "STRING"; }
    case TOKEN_TYPE::FSTRING:   { return "FSTRING"; }
    case TOKEN_TYPE::BOOL:      { return "BOOL"; }

    // OPERATORS
    case TOKEN_TYPE::PLUS:      { return "PLUS"; }
    case TOKEN_TYPE::MINUS:     { return "MINUS"; }
    case TOKEN_TYPE::TIMES:     { return "TIMES"; }
    case TOKEN_TYPE::MOD:       { return "MOD"; }
    case TOKEN_TYPE::SLASH:     { return "SLASH"; }
    case TOKEN_TYPE::PERIOD:    { return "PERIOD"; }
    case TOKEN_TYPE::NOT:       { return "NOT"; }
    case TOKEN_TYPE::ASSIGN:    { return "ASSIGN"; }
    case TOKEN_TYPE::EQ:        { return "EQ"; }
    case TOKEN_TYPE::NEQ:       { return "NEQ"; }
    case TOKEN_TYPE::LT:        { return "LT"; }
    case TOKEN_TYPE::GT:        { return "GT"; }
    case TOKEN_TYPE::LQ:        { return "LQ"; }
    case TOKEN_TYPE::GQ:        { return "GQ"; }
    case TOKEN_TYPE::ARROW:     { return "ARROW"; }
    case TOKEN_TYPE::OR:        { return "OR"; }
    case TOKEN_TYPE::AND:       { return "AND"; }

    // SEPARATORS
    case TOKEN_TYPE::LPAREN:    { return "LPAREN"; }
    case TOKEN_TYPE::RPAREN:    { return "RPAREN"; }
    case TOKEN_TYPE::LBRACK:    { return "LBRACK"; }
    case TOKEN_TYPE::RBRACK:    { return "RBRACK"; }
    case TOKEN_TYPE::LSQBRACK:  { return "LSQBRACK"; }
    case TOKEN_TYPE::RSQBRACK:  { return "RSQBRACK"; }
    case TOKEN_TYPE::SEMICOLON: { return "SEMICOLON"; }
    case TOKEN_TYPE::COLON:     { return "COLON"; }
    case TOKEN_TYPE::COMMA:     { return "COMMA"; }

    // KEYWORDS
    case TOKEN_TYPE::IF:        { return "IF"; }
    case TOKEN_TYPE::ELSE:      { return "ELSE"; }
    case TOKEN_TYPE::FOR:       { return "FOR"; }
    case TOKEN_TYPE::FN:        { return "FN"; }
    case TOKEN_TYPE::LET:       { return "LET"; }
    case TOKEN_TYPE::CONST:     { return "CONST"; }
    case TOKEN_TYPE::RETURN:    { return "RETURN"; }
    case TOKEN_TYPE::WHILE:     { return "WHILE"; }
    case TOKEN_TYPE::CLASS:     { return "CLASS"; }
    case TOKEN_TYPE::IMPORT:    { return "IMPORT"; }
    case TOKEN_TYPE::EXPORT:    { return "EXPORT"; }
    case TOKEN_TYPE::FROM:      { return "FROM"; }

    // UNIQUE
    case TOKEN_TYPE::IDENTIFIER:{ return "IDENTIFIER"; }
    case TOKEN_TYPE::TT_EOF:    { return "EOF"; }

    default: { return "Unhandled token!"; }
}
}

std::string print_token(const Token &token)
{
    if (token.value != nullptr)
        return std::format("{}:{}", ttype_to_string(token.type), token.value);
    return ttype_to_string(token.type);
}
}
