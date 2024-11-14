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

class BaseASTNode
{
public:
  virtual ~BaseASTNode() = default;

  BaseASTNode() = default;

  virtual std::string print() = 0;
  virtual int visit(Visitor& visitor) = 0;
};

class LiteralASTNode : public BaseASTNode
{
public:
  Token token;
  explicit LiteralASTNode(const Token& t)
    :BaseASTNode(), token(t) {}

  std::string print() override
  {
    return "literal:(" + std::string(token.value) + ")";
  }
  int visit(Visitor& visitor) override
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

  int visit(Visitor& visitor) override
  {
    if (visitor.interpreter.vars.contains(token.value))
      return visitor.interpreter.vars[token.value]->value;

    std::cout << "Unknown variable " << token.value << " ";
    return 0; // Encapsulate later
  }
};

class VariableASTNode : public BaseASTNode
{
public:
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

  int visit(Visitor& visitor) override
  {
    if (visitor.interpreter.vars.contains(name.value))
    {
      std::cout << "Var already defined!\n";
      return 0;
    }
    int v = 0;
    if (value != nullptr)
      v = value->visit(visitor);
    visitor.interpreter.vars[name.value] = std::make_unique<Variable>(type.type == TOKEN_TYPE::CONST, v);

    return v;
  }; // TODO
};

class UnaryOpASTNode : public BaseASTNode
{
  Token op;
  std::unique_ptr<BaseASTNode> RHS;
public:
  UnaryOpASTNode(const Token& t, std::unique_ptr<BaseASTNode> RHS)
    :BaseASTNode(), op(t), RHS(std::move(RHS)) {}

  std::string print() override
  {
    return "unary:(" + print_token(op) + " " + RHS->print() + ")";
  }

  int visit(Visitor& visitor) override
  {
    switch (op.type)
    {
      case TOKEN_TYPE::MINUS: { return -RHS->visit(visitor); }
      case TOKEN_TYPE::PLUS: { return +RHS->visit(visitor); }
      case TOKEN_TYPE::NOT: { return !RHS->visit(visitor); }
      default: { std::cout << "Unary unhandled default\n"; return 0; }
    }
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

  int visit(Visitor& visitor) override
  {
    auto lhs = LHS->visit(visitor);
    auto rhs = RHS->visit(visitor);
    if (op.type == TOKEN_TYPE::PLUS)
      return lhs + rhs;
    if (op.type == TOKEN_TYPE::TIMES)
      return lhs * rhs;
    if (op.type == TOKEN_TYPE::MINUS)
      return lhs - rhs;
    if (op.type == TOKEN_TYPE::SLASH)
      return lhs / rhs;
  }
};

class StatementASTNode : public BaseASTNode
{
  Token identifier;
  std::unique_ptr<BaseASTNode> RHS;
public:
  StatementASTNode(const Token& t, std::unique_ptr<BaseASTNode> r)
    :BaseASTNode(), identifier(t), RHS(std::move(r)) {}

  std::string print() override
  {
    return "stmnt:(" + print_token(identifier) + " " + RHS->print() + ")";
  }
  int visit(Visitor& visitor) override
  {
    if (visitor.interpreter.vars.contains(identifier.value) )
    {
      if (visitor.interpreter.vars[identifier.value]->is_constant)
      {
        std::cout << "Var is constant!\n";
        return 0;
      }
      visitor.interpreter.vars[identifier.value]->value = RHS->visit(visitor);
      return visitor.interpreter.vars[identifier.value]->value;
    }
    std::cout << "Unknown identifier " << identifier.value << "\n";
    return 0;
  }; // What about types??
};

class RootASTNode : public BaseASTNode
{
public:
  std::vector<std::unique_ptr<BaseASTNode>> nodes;
  RootASTNode()
    :BaseASTNode() {}

  std::string print() override
  {
    std::string res{"root:(\n"};
    for (const auto& i : nodes)
    {
      res += " " + i->print() + ";\n";
    }
    return res + ")";
  }

  int visit(Visitor& visitor) override
  {
    for (int i = 0; i < nodes.size(); i++)
    {
      std::cout << nodes[i]->visit(visitor) << "\n";
    }
    return 0;
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

  void advance();

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
      case TOKEN_TYPE::PLUS:
      case TOKEN_TYPE::MINUS:
      case TOKEN_TYPE::NOT: { return parse_unary(); }
    }
  }

  std::unique_ptr<BaseASTNode> parse_unary()
  {
    auto op = m_tokens[m_pos];
    advance();
    std::unique_ptr<BaseASTNode> expr;
    if (m_tokens[m_pos].type == TOKEN_TYPE::LPAREN)
      expr = parse_paren_expr();
    else
      expr = parse_primary_expr();

    return std::make_unique<UnaryOpASTNode>(op, std::move(expr));
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

  // statement :: identifier = expr
  // statement :: identifier
  std::unique_ptr<BaseASTNode> parse_statement_or_ident()
  {
    const auto& identifier = m_tokens[m_pos];
    advance(); // eat id
    if (m_tokens[m_pos].type != TOKEN_TYPE::EQ)
    {
      m_pos--;
      return parse_expr();
      // return std::make_unique<IdentifierASTNode>(identifier); // if there is no =, then this is just an identifier, e.g. 1 + foo
      // std::cout << "Expected eq\n";
      // return nullptr;
    }
    advance(); // eat eq
    auto expr = parse_expr();

    std::unique_ptr<StatementASTNode> stmnt = std::make_unique<StatementASTNode>(identifier, std::move(expr));
    return std::move(stmnt);
  }

  // root = (var_decl | expr | ... ;)*
  std::unique_ptr<BaseASTNode> parse_root()
  {
    auto root = std::make_unique<RootASTNode>();
    while (m_pos < m_tokens.size())
    {
      switch (m_tokens[m_pos].type)
      {
        case TOKEN_TYPE::TT_EOF: { return root; }
        case TOKEN_TYPE::VAR:
        case TOKEN_TYPE::CONST:
        case TOKEN_TYPE::LET: { root->nodes.push_back(std::move(parse_var_decl())); break; }
        case TOKEN_TYPE::IDENTIFIER: { root->nodes.push_back(std::move(parse_statement_or_ident())); break; }
        default: { root->nodes.push_back(std::move(parse_expr())); break; }
      }
      if (m_pos != m_tokens.size() && m_tokens[m_pos].type != TOKEN_TYPE::SEMICOLON)
      {
        std::cout << "Excpected a semicolon;\n";
        return nullptr;
      }
      advance(); // eat ;
    }
    return root;
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

