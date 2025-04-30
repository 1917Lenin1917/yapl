//
// Created by lenin on 17.11.2024.
//

#pragma once

#include <string>
#include <iostream>
#include <format>
#include <functional>
#include <memory>
#include <utility>

#include "Token.hpp"
#include "Visitor.hpp"
#include "Function.hpp"
#include "values/Value.hpp"

#define REPEAT(n, c) std::string(n, c)

namespace yapl {
class Visitor;
class Function;
class Value;

class BaseASTNode
{
public:
  virtual ~BaseASTNode() = default;

  BaseASTNode() = default;

  virtual std::string print(size_t indent_size) = 0;
  virtual std::shared_ptr<Value> visit(Visitor &visitor) = 0;
};

class LiteralASTNode final : public BaseASTNode
{
public:
  Token token;
  explicit LiteralASTNode(const Token& t)
    :BaseASTNode(), token(t) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class IdentifierASTNode final : public BaseASTNode
{
public:
  Token token;

  explicit IdentifierASTNode(const Token& t)
    :BaseASTNode(), token(t) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class IndexASTNode final : public BaseASTNode
{
public:
  std::unique_ptr<BaseASTNode> base_expr;
  std::unique_ptr<BaseASTNode> index_expr;

  IndexASTNode(std::unique_ptr<BaseASTNode> base_expr, std::unique_ptr<BaseASTNode> index_expr)
    :BaseASTNode(), base_expr(std::move(base_expr)), index_expr(std::move(index_expr)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class ArrayASTNode final : public BaseASTNode
{
public:
  std::vector<std::unique_ptr<BaseASTNode>> values;

  explicit ArrayASTNode(std::vector<std::unique_ptr<BaseASTNode>>& values)
    :BaseASTNode(), values(std::move(values)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class VariableASTNode final : public BaseASTNode
{
public:
  Token type;
  Token name;
  std::unique_ptr<BaseASTNode> value;
public:
  explicit VariableASTNode(const Token& t, const Token& n, std::unique_ptr<BaseASTNode> v = nullptr)
    : BaseASTNode(), type(t), name(n), value(std::move(v)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class UnaryOpASTNode final : public BaseASTNode
{
public:
  Token op;
  std::unique_ptr<BaseASTNode> RHS;
  UnaryOpASTNode(const Token& t, std::unique_ptr<BaseASTNode> RHS)
    :BaseASTNode(), op(t), RHS(std::move(RHS)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class BinaryOpASTNode final : public BaseASTNode
{
public:
    Token op;
    std::unique_ptr<BaseASTNode> LHS, RHS;
  BinaryOpASTNode(const Token& token, std::unique_ptr<BaseASTNode> LHS, std::unique_ptr<BaseASTNode> RHS)
    :BaseASTNode(), op(token), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class StatementASTNode final : public BaseASTNode
{
public:
    Token identifier;
    std::unique_ptr<BaseASTNode> RHS;
  StatementASTNode(const Token& t, std::unique_ptr<BaseASTNode> r)
    :BaseASTNode(), identifier(t), RHS(std::move(r)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class StatementIndexASTNode final : public BaseASTNode
{
public:
  std::unique_ptr<BaseASTNode> identifier; // IndexASTNode
  std::unique_ptr<BaseASTNode> RHS;
  StatementIndexASTNode(std::unique_ptr<BaseASTNode> i, std::unique_ptr<BaseASTNode> r)
    :BaseASTNode(), identifier(std::move(i)), RHS(std::move(r)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class FunctionArgumentASTNode final : public BaseASTNode
{
public:
  Token name;
  Token type;
  bool is_args, is_kwargs;

  FunctionArgumentASTNode(const Token& n, const Token& t, bool is_args = false, bool is_kwargs = false)
    :BaseASTNode(), name(n), type(t), is_args(is_args), is_kwargs(is_kwargs) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class FunctionArgumentListASTNode final : public BaseASTNode
{
  int m_arg_amount = -1;
public:
  std::vector<std::unique_ptr<FunctionArgumentASTNode>> args;
  std::unique_ptr<FunctionArgumentASTNode> args_arg, kwargs_arg;
  explicit FunctionArgumentListASTNode(std::vector<std::unique_ptr<FunctionArgumentASTNode>>& args)
    :BaseASTNode(), m_arg_amount(args.size()), args(std::move(args)) {}
  explicit FunctionArgumentListASTNode(std::vector<std::unique_ptr<FunctionArgumentASTNode>>&& args)
    :BaseASTNode(), m_arg_amount(args.size()), args(std::move(args)) {}

  explicit FunctionArgumentListASTNode(std::vector<std::unique_ptr<FunctionArgumentASTNode>>& args, std::unique_ptr<FunctionArgumentASTNode> args_arg, std::unique_ptr<FunctionArgumentASTNode> kwargs_arg)
    :BaseASTNode(), m_arg_amount(args.size()), args(std::move(args)), args_arg(std::move(args_arg)), kwargs_arg(std::move(kwargs_arg)) {}
  explicit FunctionArgumentListASTNode(std::vector<std::unique_ptr<FunctionArgumentASTNode>>&& args, std::unique_ptr<FunctionArgumentASTNode> args_arg, std::unique_ptr<FunctionArgumentASTNode> kwargs_arg)
    :BaseASTNode(), m_arg_amount(args.size()), args(std::move(args)), args_arg(std::move(args_arg)), kwargs_arg(std::move(kwargs_arg)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;

  [[nodiscard]] int get_argument_amount() const { return m_arg_amount; }
  [[nodiscard]] std::string get_argument_name(size_t idx) const
  {
    if (m_arg_amount == -1)
      return "argument_" + std::to_string(idx);
    return args[idx]->name.value;
  }
};

class FunctionDeclASTNode final : public BaseASTNode
{
public:
  Token name;
  std::unique_ptr<BaseASTNode> args;
  Token return_type;

  FunctionDeclASTNode(const Token& n, std::unique_ptr<BaseASTNode> args, const Token& rt)
    :BaseASTNode(), name(n), args(std::move(args)), return_type(rt) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class MethodCallASTNode final : public BaseASTNode
{
public:
    std::unique_ptr<BaseASTNode> base_expr;
	Token name;
	std::vector<std::unique_ptr<BaseASTNode>> args;
	MethodCallASTNode(std::unique_ptr<BaseASTNode> base_expr, const Token& nm, std::vector<std::unique_ptr<BaseASTNode>>& args)
		:BaseASTNode(), base_expr(std::move(base_expr)), name(nm), args(std::move(args)) {}

	std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};
class FunctionCallASTNode final : public BaseASTNode
{
public:
	Token name;
	std::vector<std::unique_ptr<BaseASTNode>> args;
	FunctionCallASTNode(const Token& id, std::vector<std::unique_ptr<BaseASTNode>>& args)
		:BaseASTNode(), name(id), args(std::move(args)) {}

	std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class ReturnStatementASTNode final : public BaseASTNode
{
public:
	std::unique_ptr<BaseASTNode> expr;
	explicit ReturnStatementASTNode(std::unique_ptr<BaseASTNode> e)
		:expr(std::move(e)) {}

	std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class ScopeASTNode final : public BaseASTNode
{
public:
  std::vector<std::unique_ptr<BaseASTNode>> nodes;
  ScopeASTNode()
    :BaseASTNode() {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class FunctionASTNode final : public BaseASTNode
{
public:
  std::unique_ptr<BaseASTNode> decl;
  std::unique_ptr<BaseASTNode> body;

  FunctionASTNode(std::unique_ptr<BaseASTNode> decl, std::unique_ptr<BaseASTNode> body)
    :BaseASTNode(), decl(std::move(decl)), body(std::move(body)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class BuiltinCustomVisitFunctionASTNode final : public BaseASTNode
{
public:
  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;

  std::function<std::shared_ptr<Value>(std::shared_ptr<Function>)> func;

  explicit BuiltinCustomVisitFunctionASTNode(std::function<std::shared_ptr<Value>(std::shared_ptr<Function>)> func)
    :BaseASTNode(), func(std::move(func)) {}
};
class IfElseExpressionASTNode final : public BaseASTNode
{
public:
  std::unique_ptr<BaseASTNode> condition;
  std::unique_ptr<BaseASTNode> true_scope;
  std::unique_ptr<BaseASTNode> false_scope;

  IfElseExpressionASTNode(std::unique_ptr<BaseASTNode> cond, std::unique_ptr<BaseASTNode> ts, std::unique_ptr<BaseASTNode> fs = nullptr)
    :BaseASTNode(), condition(std::move(cond)), true_scope(std::move(ts)), false_scope(std::move(fs)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class WhileLoopASTNode final : public BaseASTNode
{
public:
  std::unique_ptr<BaseASTNode> condition;
  std::unique_ptr<BaseASTNode> scope;

  WhileLoopASTNode(std::unique_ptr<BaseASTNode> condition, std::unique_ptr<BaseASTNode> scope)
    :BaseASTNode(), condition(std::move(condition)), scope(std::move(scope)) {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};
class ForLoopASTNode final : public BaseASTNode
{
public:
  std::unique_ptr<BaseASTNode> declaration;
  std::unique_ptr<BaseASTNode> condition;
  std::unique_ptr<BaseASTNode> increment;

  std::unique_ptr<BaseASTNode> scope;

  ForLoopASTNode(std::unique_ptr<BaseASTNode> decl, std::unique_ptr<BaseASTNode> cond, std::unique_ptr<BaseASTNode> inc, std::unique_ptr<BaseASTNode> scope)
    :BaseASTNode(), declaration(std::move(decl)), condition(std::move(cond)), increment(std::move(inc)), scope(std::move(scope)){}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class StarredExpressionASTNode final : public BaseASTNode
{
public:
    std::unique_ptr<BaseASTNode> expression;
    explicit StarredExpressionASTNode(std::unique_ptr<BaseASTNode> expr)
        :BaseASTNode(), expression(std::move(expr)) {}

    std::string print(size_t indent_size) override;
    std::shared_ptr<Value> visit(Visitor &visitor) override;
};

class RootASTNode final : public BaseASTNode
{
public:
  std::vector<std::unique_ptr<BaseASTNode>> nodes;
  RootASTNode()
    :BaseASTNode() {}

  std::string print(size_t indent_size) override;

  std::shared_ptr<Value> visit(Visitor &visitor) override;
};
}

