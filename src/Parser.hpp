//
// Created by lenin on 14.11.2024.
//

#pragma once
#include <iostream>
#include <memory>
#include <vector>

#include "Token.hpp"
#include "Interpreter.hpp"

using namespace yapl;

namespace yapl {



class Parser
{
private:
  std::vector<Token> m_tokens;
  size_t m_pos;

public:
  explicit Parser(const std::vector<Token>& tokens)
    :m_tokens(tokens), m_pos(0) {}

  void advance();

  std::unique_ptr<BaseASTNode> parse_literal();
  std::unique_ptr<BaseASTNode> parse_identifier();
  std::unique_ptr<BaseASTNode> parse_array();
  std::unique_ptr<BaseASTNode> parse_primary_expr();
  std::unique_ptr<BaseASTNode> parse_unary();
  std::unique_ptr<BaseASTNode> parse_paren_expr();
  std::unique_ptr<BaseASTNode> parse_expr();
  std::unique_ptr<BaseASTNode> parse_semic_expr();
	std::unique_ptr<BaseASTNode> parse_return();
  std::unique_ptr<BaseASTNode> parse_binop_rhs(int expr_prec, std::unique_ptr<BaseASTNode> LHS);
  std::vector<std::unique_ptr<BaseASTNode>> parse_var_decl();
  std::unique_ptr<BaseASTNode> parse_for_loop();
  std::unique_ptr<BaseASTNode> parse_while_loop();
  // statement :: identifier = expr
  // statement :: identifier
  // statement :: identifier()
  // statement :: identifier(args...)
  std::unique_ptr<BaseASTNode> parse_statement_or_ident();
  std::unique_ptr<BaseASTNode> parse_function_arguments();
  std::unique_ptr<BaseASTNode> parse_function_declaration();
  std::unique_ptr<BaseASTNode> parse_ifelse_statement();
  std::unique_ptr<BaseASTNode> parse_scope();
  std::unique_ptr<BaseASTNode> parse_function();

  // root = (var_decl | expr | ... ;)*
  std::unique_ptr<BaseASTNode> parse_root();

  //
  int get_token_precedence(const Token& t)
  {
    switch (t.type)
    {
      case TOKEN_TYPE::LT: { return 10; }
      case TOKEN_TYPE::GT: { return 10; }
      case TOKEN_TYPE::LQ: { return 10; }
      case TOKEN_TYPE::GQ: { return 10; }
      case TOKEN_TYPE::EQ: { return 10; }
      case TOKEN_TYPE::PLUS: { return 20; }
      case TOKEN_TYPE::MINUS: { return 20; }
      case TOKEN_TYPE::TIMES: { return 40; }
      case TOKEN_TYPE::SLASH: { return 40; }
      default: { return -1; }
    }
  }
};

}

