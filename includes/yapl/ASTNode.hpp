#pragma once

#include <string>
#include <iostream>
#include <format>
#include <memory>
#include <utility>
#include <cstddef>
#include <vector>

#include "Token.hpp"
#include "BaseVisitor.hpp"
#include "ByteCodeVisitor.hpp"

#define REPEAT(n, c) std::string(n, c)

namespace yapl {
class ByteCodeVisitor;
class Function;
class Value;

struct SourcePosition
{
  std::size_t line;
  std::size_t character;
};

struct SourceLocation
{
  SourcePosition start;
  SourcePosition end;
};

class BaseASTNode;
using ASTPtr = std::unique_ptr<BaseASTNode>;

class BaseASTNode
{
public:
  std::size_t id;
  SourceLocation location;

  virtual ~BaseASTNode() = default;

  explicit BaseASTNode(const std::size_t id, const SourceLocation location)
    : id(id), location(location) {}

  virtual std::string print(size_t indent_size) = 0;

  virtual void visit(Visitor &visitor) {}
};

class IntegerASTNode final : public BaseASTNode
{
public:
  int value;

  explicit IntegerASTNode(const Token &token, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), value(std::stoi(token.value)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class FloatASTNode final : public BaseASTNode
{
public:
  float value;

  explicit FloatASTNode(const Token &token, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), value(std::stof(token.value)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class BooleanASTNode final : public BaseASTNode
{
public:
  bool value;

  explicit BooleanASTNode(const Token &token, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), value(token.value == std::string("true")) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class StringASTNode final : public BaseASTNode
{
public:
  std::string value;

  explicit StringASTNode(const Token &token, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), value(token.value) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class IdentifierASTNode final : public BaseASTNode
{
public:
  Token token;

  explicit IdentifierASTNode(const Token &token, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), token(token) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class IndexASTNode final : public BaseASTNode
{
public:
  ASTPtr base_expr;
  ASTPtr index_expr;

  IndexASTNode(ASTPtr base_expr, ASTPtr index_expr, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), base_expr(std::move(base_expr)), index_expr(std::move(index_expr)) {}

  std::string print(size_t indent_size) override;
  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ArrayASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> values;

  explicit ArrayASTNode(std::vector<ASTPtr> &values, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), values(std::move(values)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class DictASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> keys;
  std::vector<ASTPtr> values;

  explicit DictASTNode(
    std::vector<ASTPtr> &&keys,
    std::vector<ASTPtr> &&values,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), keys(std::move(keys)), values(std::move(values)) {}

  std::string print(size_t indent_size) override;
  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ClassASTNode final : public BaseASTNode
{
public:
  Token name;
  std::vector<ASTPtr> member_functions;

  explicit ClassASTNode(
    const Token &name,
    std::vector<ASTPtr> &&member_functions,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), name(name), member_functions(std::move(member_functions)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class VariableASTNode final : public BaseASTNode
{
public:
  Token type;
  Token name;
  ASTPtr value;

  explicit VariableASTNode(
    const Token &type,
    const Token &name,
    ASTPtr value,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), type(type), name(name), value(std::move(value)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class UnaryOpASTNode final : public BaseASTNode
{
public:
  Token op;
  ASTPtr RHS;

  UnaryOpASTNode(const Token &op, ASTPtr rhs, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), op(op), RHS(std::move(rhs)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class BinaryOpASTNode final : public BaseASTNode
{
public:
  Token op;
  ASTPtr LHS;
  ASTPtr RHS;

  BinaryOpASTNode(const Token &op, ASTPtr lhs, ASTPtr rhs, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), op(op), LHS(std::move(lhs)), RHS(std::move(rhs)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class StatementASTNode final : public BaseASTNode
{
public:
  ASTPtr base;
  ASTPtr RHS;

  StatementASTNode(ASTPtr base, ASTPtr rhs, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), base(std::move(base)), RHS(std::move(rhs)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ImportASTNode final : public BaseASTNode
{
public:
  std::vector<Token> identifiers;
  ASTPtr module;

  ImportASTNode(
    std::vector<Token> &&identifiers,
    ASTPtr module,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), identifiers(std::move(identifiers)), module(std::move(module)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ExportASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> variables;

  explicit ExportASTNode(std::vector<ASTPtr> &&variables, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), variables(std::move(variables)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class StatementIndexASTNode final : public BaseASTNode
{
public:
  ASTPtr identifier;
  ASTPtr RHS;

  StatementIndexASTNode(ASTPtr identifier, ASTPtr rhs, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), identifier(std::move(identifier)), RHS(std::move(rhs)) {}

  std::string print(size_t indent_size) override;
  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class FunctionArgumentASTNode final : public BaseASTNode
{
public:
  Token name;
  Token type;
  bool is_args;
  bool is_kwargs;
  bool is_keyword;

  FunctionArgumentASTNode(
    const Token &name,
    const Token &type,
    bool is_args,
    bool is_kwargs,
    bool is_keyword,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location),
      name(name),
      type(type),
      is_args(is_args),
      is_kwargs(is_kwargs),
      is_keyword(is_keyword) {}

  std::string print(size_t indent_size) override;
};

class FunctionArgumentListASTNode final : public BaseASTNode
{
public:
  std::vector<std::unique_ptr<FunctionArgumentASTNode>> args;
  std::unique_ptr<FunctionArgumentASTNode> args_arg;
  std::unique_ptr<FunctionArgumentASTNode> kwargs_arg;

  explicit FunctionArgumentListASTNode(
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> &args,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), args(std::move(args)) {}

  explicit FunctionArgumentListASTNode(
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> &&args,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), args(std::move(args)) {}

  explicit FunctionArgumentListASTNode(
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> &args,
    std::unique_ptr<FunctionArgumentASTNode> args_arg,
    std::unique_ptr<FunctionArgumentASTNode> kwargs_arg,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location),
      args(std::move(args)),
      args_arg(std::move(args_arg)),
      kwargs_arg(std::move(kwargs_arg)) {}

  explicit FunctionArgumentListASTNode(
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> &&args,
    std::unique_ptr<FunctionArgumentASTNode> args_arg,
    std::unique_ptr<FunctionArgumentASTNode> kwargs_arg,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location),
      args(std::move(args)),
      args_arg(std::move(args_arg)),
      kwargs_arg(std::move(kwargs_arg)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class FunctionDeclASTNode final : public BaseASTNode
{
public:
  Token name;
  ASTPtr args;
  Token return_type;

  FunctionDeclASTNode(
    const Token &name,
    ASTPtr args,
    const Token &return_type,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), name(name), args(std::move(args)), return_type(return_type) {}

  std::string print(size_t indent_size) override;
};

class GetPropertyASTNode final : public BaseASTNode
{
public:
  ASTPtr base_expr;
  Token name;

  GetPropertyASTNode(ASTPtr base_expr, const Token &name, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), base_expr(std::move(base_expr)), name(name) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class SetPropertyASTNode final : public BaseASTNode
{
public:
  ASTPtr base_expr;
  Token name;
  ASTPtr RHS;

  SetPropertyASTNode(
    ASTPtr base_expr,
    const Token &name,
    ASTPtr RHS,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), base_expr(std::move(base_expr)), name(name), RHS(std::move(RHS)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class MethodCallASTNode final : public BaseASTNode
{
public:
  ASTPtr base_expr;
  Token name;
  std::vector<ASTPtr> args;

  MethodCallASTNode(
    ASTPtr base_expr,
    const Token &name,
    std::vector<ASTPtr> &args,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), base_expr(std::move(base_expr)), name(name), args(std::move(args)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class FunctionCallASTNode final : public BaseASTNode
{
public:
  ASTPtr base;
  std::vector<ASTPtr> args;

  FunctionCallASTNode(ASTPtr base, std::vector<ASTPtr> &args, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), base(std::move(base)), args(std::move(args)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ReturnStatementASTNode final : public BaseASTNode
{
public:
  ASTPtr expr;

  explicit ReturnStatementASTNode(ASTPtr expr, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), expr(std::move(expr)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ScopeASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> nodes;

  explicit ScopeASTNode(const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class FunctionASTNode final : public BaseASTNode
{
public:
  ASTPtr decl;
  ASTPtr body;

  FunctionASTNode(ASTPtr decl, ASTPtr body, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), decl(std::move(decl)), body(std::move(body)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class IfElseExpressionASTNode final : public BaseASTNode
{
public:
  ASTPtr condition;
  ASTPtr true_scope;
  ASTPtr false_scope;

  IfElseExpressionASTNode(
    ASTPtr condition,
    ASTPtr true_scope,
    ASTPtr false_scope,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location),
      condition(std::move(condition)),
      true_scope(std::move(true_scope)),
      false_scope(std::move(false_scope)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class WhileLoopASTNode final : public BaseASTNode
{
public:
  ASTPtr condition;
  ASTPtr scope;

  WhileLoopASTNode(ASTPtr condition, ASTPtr scope, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), condition(std::move(condition)), scope(std::move(scope)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ForLoopASTNode final : public BaseASTNode
{
public:
  ASTPtr declaration;
  ASTPtr condition;
  ASTPtr increment;
  ASTPtr scope;

  ForLoopASTNode(
    ASTPtr declaration,
    ASTPtr condition,
    ASTPtr increment,
    ASTPtr scope,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location),
      declaration(std::move(declaration)),
      condition(std::move(condition)),
      increment(std::move(increment)),
      scope(std::move(scope)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ForEachLoopASTNode final : public BaseASTNode
{
public:
  Token identifier;
  ASTPtr iterable_expr;
  ASTPtr scope;

  ForEachLoopASTNode(
    const Token &identifier,
    ASTPtr iterable_expr,
    ASTPtr scope,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location),
      identifier(identifier),
      iterable_expr(std::move(iterable_expr)),
      scope(std::move(scope)) {}

  std::string print(size_t indent_size) override;
};

class KeyParamExpressionASTNode final : public BaseASTNode
{
public:
  Token identifier;
  ASTPtr expression;

  explicit KeyParamExpressionASTNode(
    const Token &identifier,
    ASTPtr expression,
    const std::size_t id,
    const SourceLocation location
  )
    : BaseASTNode(id, location), identifier(identifier), expression(std::move(expression)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class StarredExpressionASTNode final : public BaseASTNode
{
public:
  ASTPtr expression;

  explicit StarredExpressionASTNode(ASTPtr expression, const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location), expression(std::move(expression)) {}

  std::string print(size_t indent_size) override;
};

class RootASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> nodes;

  explicit RootASTNode(const std::size_t id, const SourceLocation location)
    : BaseASTNode(id, location) {}

  std::string print(size_t indent_size) override;

  void visit(ByteCodeVisitor &visitor) { visitor.visit_RootASTNode(*this); }
};
}
