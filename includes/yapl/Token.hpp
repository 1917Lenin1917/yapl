//
// Created by lenin on 13.11.2024.
//

#pragma once
#include <string>
#include <cassert>
#include <format>
#include <iostream>
#include <cstring>

#include "Position.hpp"

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
  LET,
  CONST,
  RETURN,
  WHILE,
  CLASS,
  IMPORT,
  EXPORT,
  FROM,

  // UNIQUE
  IDENTIFIER,
  TT_EOF,
};

struct Token
{
  TOKEN_TYPE type = TOKEN_TYPE::DEFAULT;
  std::string value;

  Range range;
};

std::string print_token_type(TOKEN_TYPE tt);
std::string print_token(const Token& token);
}
