//
// Created by lenin on 14.11.2024.
//

#pragma once
#include <iostream>
#include <memory>
#include <vector>

#include "Token.hpp"


namespace yapl {

class BaseASTNode
{
public:
  BaseASTNode() = default;

  virtual std::string print() = 0;
  virtual int evaluate() = 0;
};

class LiteralASTNode : public BaseASTNode
{
  Token token;
public:
  explicit LiteralASTNode(const Token& t)
    :BaseASTNode(), token(t) {}

  std::string print() override
  {
    return "literal:(" + std::string(token.value) + ")";
  }
  int evaluate() override
  {
    // TODO: add more types
    return std::stoi(token.value);
  }
};

class IdentifierASTNode : public BaseASTNode
{
  Token token;
public:
  explicit IdentifierASTNode(const Token& t)
    :BaseASTNode(), token(t) {}

  std::string print() override
  {
    return "identifier:(" + std::string(token.value) + ")";
  }

  int evaluate() override { return 0; }
};

class VariableASTNode : public BaseASTNode
{
  Token type;
  Token name;
  std::unique_ptr<BaseASTNode> value;
public:
  explicit VariableASTNode(const Token& t, const Token& n, std::unique_ptr<BaseASTNode> v = nullptr)
    : BaseASTNode(), type(t), name(n), value(std::move(v)) {}

  std::string print() override
  {
    if (value == nullptr)
      return "variable:(" + print_token(type) + " " + name.value + ")";
    return "variable:(" + print_token(type) + " " + name.value + " = " + value->print() + ")";
  }

  int evaluate() override { return 0; }; // TODO
};

class UnaryOpASTNode : public BaseASTNode
{
  Token op;
  std::unique_ptr<BaseASTNode> RHS;

  std::string print() override
  {
    return "unary:(" + print_token(op) + " " + RHS->print() + ")";
  }

  int evaluate() override
  {
    if (op.type == TOKEN_TYPE::MINUS)
      return -RHS->evaluate();
    return 0; // ???
  }
};

class BinaryOpASTNode : public BaseASTNode
{
  Token op;
  std::unique_ptr<BaseASTNode> LHS, RHS;
public:
  BinaryOpASTNode(const Token& token, std::unique_ptr<BaseASTNode> LHS, std::unique_ptr<BaseASTNode> RHS)
    :BaseASTNode(), op(token), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  std::string print() override
  {
    return "binary:(" + LHS->print() + " " + print_token(op) + " " + RHS->print() + ")";
  }

  int evaluate() override
  {
    auto lhs = LHS->evaluate();
    auto rhs = RHS->evaluate();
    if (op.type == TOKEN_TYPE::PLUS)
      return LHS->evaluate() + RHS->evaluate();
    if (op.type == TOKEN_TYPE::TIMES)
      return LHS->evaluate() * RHS->evaluate();
    if (op.type == TOKEN_TYPE::MINUS)
      return LHS->evaluate() - RHS->evaluate();
    if (op.type == TOKEN_TYPE::SLASH)
      return LHS->evaluate() / RHS->evaluate();
  }
};

class Parser
{
private:
  std::vector<Token> m_tokens;
  size_t m_pos;

public:
  explicit Parser(const std::vector<Token>& tokens)
    :m_tokens(tokens), m_pos(0) {}

  std::unique_ptr<BaseASTNode> generate_ast();
  Token advance();

  std::unique_ptr<BaseASTNode> parse_literal()
  {
    auto res = std::make_unique<LiteralASTNode>(m_tokens[m_pos]);
    advance();
    return res;
  }

  std::unique_ptr<BaseASTNode> parse_identifier()
  {
    auto res = std::make_unique<IdentifierASTNode>(m_tokens[m_pos]);
    advance();
    return res;
  }

  std::unique_ptr<BaseASTNode> parse_primary_expr()
  {
    switch (m_tokens[m_pos].type)
    {
      default:
      {
        std::cout << "Unknown token when expecting an expression!\n";
        return nullptr;
      }
      case TOKEN_TYPE::INTEGER:
      case TOKEN_TYPE::FLOAT:
      case TOKEN_TYPE::STRING:
      case TOKEN_TYPE::FSTRING:
      case TOKEN_TYPE::BOOL: { return parse_literal(); }
      case TOKEN_TYPE::IDENTIFIER: { return parse_identifier(); }
      case TOKEN_TYPE::LPAREN: { return parse_paren_expr(); }

    }
  }

  std::unique_ptr<BaseASTNode> parse_paren_expr()
  {
    advance();
    auto expr = parse_expr();
    if (expr == nullptr)
      return nullptr;

    if (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
    {
      // TODO: add better logger
      std::cerr << "Expected a ) token at " << __FILE__ << " " <<  __LINE__ << " " << __func__ << "\n";
      return nullptr;
    }
    advance();
    return expr;
  }

  std::unique_ptr<BaseASTNode> parse_expr()
  {
    // TODO: change
    auto LHS = parse_primary_expr();

    if (LHS == nullptr)
      return nullptr;

    return parse_binop_rhs(0, std::move(LHS));
  };

  // binoprhs = (op primary)*
  std::unique_ptr<BaseASTNode> parse_binop_rhs(int expr_prec, std::unique_ptr<BaseASTNode> LHS)
  {
    // tokens that arent operators have precedence of -1, so if expr doesnt have a RHS, we will just exit the loop
    while (true)
    {
      auto token_prec = get_token_precedence(m_tokens[m_pos]);
      if (token_prec < expr_prec)
        return LHS;

      auto op = m_tokens[m_pos];
      advance();

      auto RHS = parse_primary_expr();
      if (!RHS)
        return nullptr;

      // if next op has higher prec, we let it take the current rhs as its lhs
      // 1 + 2 * 3
      // LHS = 1
      // OP = + ; prec = 20
      // RHS = 2
      // NEXT_OP = * ; prec = 40
      // => we should pass 2 to * operator

      int next_prec = get_token_precedence(m_tokens[m_pos]);
      if (token_prec < next_prec)
      {
        RHS = parse_binop_rhs(token_prec + 1, std::move(RHS));
        if (RHS == nullptr)
          return nullptr;
      }

      LHS = std::make_unique<BinaryOpASTNode>(op, std::move(LHS), std::move(RHS));
    }
  }

  std::unique_ptr<BaseASTNode> parse_var_decl()
  {
    auto decl_token = m_tokens[m_pos];
    advance();
    auto name_identifier = m_tokens[m_pos];
    advance();

    if (m_tokens[m_pos].type != TOKEN_TYPE::EQ)
      return std::make_unique<VariableASTNode>(decl_token, name_identifier);

    advance();
    auto expr = parse_expr();
    return std::make_unique<VariableASTNode>(decl_token, name_identifier, std::move(expr));
  }

  //
  int get_token_precedence(const Token& t)
  {
    switch (t.type)
    {
      case TOKEN_TYPE::PLUS: { return 20; }
      case TOKEN_TYPE::MINUS: { return 20; }
      case TOKEN_TYPE::TIMES: { return 40; }
      case TOKEN_TYPE::SLASH: { return 40; }
      default: { return -1; }
    }
  }
};

}

