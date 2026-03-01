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
#include "BaseVisitor.hpp"
#include "ByteCodeVisitor.hpp"
#include "values/Value.hpp"

#define REPEAT(n, c) std::string(n, c)

namespace yapl {
class ByteCodeVisitor;
class Function;
class Value;

class BaseASTNode;
using ASTPtr = std::unique_ptr<BaseASTNode>;

class BaseASTNode
{
public:
  std::size_t id;

  virtual ~BaseASTNode() = default;

  explicit BaseASTNode(const std::size_t id);

  virtual std::string print(size_t indent_size) = 0;

  virtual void visit(Visitor &visitor) {}
};

class IntegerASTNode final : public BaseASTNode
{
public:
    int value;
    explicit IntegerASTNode(const Token& t, const std::size_t id)
            :BaseASTNode(id), value(std::stoi(t.value)) { }

    std::string print(size_t indent_size) override;

    void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class FloatASTNode final : public BaseASTNode
{
public:
    float value;
    explicit FloatASTNode(const Token& t, const std::size_t id)
            :BaseASTNode(id)
    {
        value = std::stof(t.value);
    }

    std::string print(size_t indent_size) override;

  
    void visit(Visitor &visitor) override { visitor.visit(*this); }
};
class BooleanASTNode final : public BaseASTNode
{
public:
    bool value;
    explicit BooleanASTNode(const Token& t, const std::size_t id)
            :BaseASTNode(id)
    {
        value = t.value == std::string("true");
    }

    std::string print(size_t indent_size) override;

  
    void visit(Visitor &visitor) override { visitor.visit(*this); }
};
class StringASTNode final : public BaseASTNode
{
public:
    std::string value;
    explicit StringASTNode(const Token& t, const std::size_t id)
            :BaseASTNode(id), value(t.value) {}

    std::string print(size_t indent_size) override;

  
    void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class IdentifierASTNode final : public BaseASTNode
{
public:
  Token token;

  explicit IdentifierASTNode(const Token& t, const std::size_t id)
    :BaseASTNode(id), token(t) {}

  std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class IndexASTNode final : public BaseASTNode
{
public:
  ASTPtr base_expr;
  ASTPtr index_expr;

  IndexASTNode(ASTPtr base_expr, ASTPtr index_expr, const std::size_t id)
    :BaseASTNode(id), base_expr(std::move(base_expr)), index_expr(std::move(index_expr)) {}

  std::string print(size_t indent_size) override;


};

class ArrayASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> values;

  explicit ArrayASTNode(std::vector<ASTPtr>& values, const std::size_t id)
    :BaseASTNode(id), values(std::move(values)) {}

  std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class DictASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> keys;
  std::vector<ASTPtr> values;

  explicit DictASTNode(std::vector<ASTPtr>&& keys, std::vector<ASTPtr>&& values, const std::size_t id)
    :BaseASTNode(id), keys(std::move(keys)), values(std::move(values)) { }

  std::string print(size_t indent_size) override;

};

class ClassASTNode final : public BaseASTNode
{
public:
  Token name;
  std::vector<ASTPtr> member_functions;

  explicit ClassASTNode(const Token &name, std::vector<ASTPtr>&& member_functions, const std::size_t id)
    :BaseASTNode(id), name(name), member_functions(std::move(member_functions)) {}

  std::string print(size_t indent_size) override;
  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class VariableASTNode final : public BaseASTNode
{
public:
  Token type;
  Token name;
  ASTPtr value;
public:
  explicit VariableASTNode(const Token& t, const Token& n, ASTPtr v, const std::size_t id)
    : BaseASTNode(id), type(t), name(n), value(std::move(v)) {}

  std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class UnaryOpASTNode final : public BaseASTNode
{
public:
  Token op;
  ASTPtr RHS;
  UnaryOpASTNode(const Token& t, ASTPtr RHS, const std::size_t id)
    :BaseASTNode(id), op(t), RHS(std::move(RHS)) {}

  std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class BinaryOpASTNode final : public BaseASTNode
{
public:
    Token op;
    ASTPtr LHS, RHS;
  BinaryOpASTNode(const Token& token, ASTPtr LHS, ASTPtr RHS, const std::size_t id)
    :BaseASTNode(id), op(token), LHS(std::move(LHS)), RHS(std::move(RHS)) {}

  std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class StatementASTNode final : public BaseASTNode
{
public:
    ASTPtr base;
    ASTPtr RHS;
  StatementASTNode(ASTPtr base, ASTPtr r, const std::size_t id)
    :BaseASTNode(id), base(std::move(base)), RHS(std::move(r)) {}

  std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ImportASTNode final : public BaseASTNode
{
public:
  std::vector<Token> identifiers;
  ASTPtr module;

  ImportASTNode(std::vector<Token>&& ids, ASTPtr module, const std::size_t id)
    :BaseASTNode(id), identifiers(std::move(ids)), module(std::move(module)) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};
class ExportASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> variables;

  explicit ExportASTNode(std::vector<ASTPtr>&& vars, const std::size_t id)
    :BaseASTNode(id), variables(std::move(vars)) {}

  std::string print(size_t indent_size) override;
  void visit(Visitor &visitor) override { visitor.visit(*this); }
};


class StatementIndexASTNode final : public BaseASTNode
{
public:
  ASTPtr identifier; // IndexASTNode
  ASTPtr RHS;
  StatementIndexASTNode(ASTPtr i, ASTPtr r, const std::size_t id)
    :BaseASTNode(id), identifier(std::move(i)), RHS(std::move(r)) {}

  std::string print(size_t indent_size) override;


};

class FunctionArgumentASTNode final : public BaseASTNode
{
public:
  Token name;
  Token type;
  bool is_args, is_kwargs, is_keyword;

  FunctionArgumentASTNode(const Token& n, const Token& t, bool is_args, bool is_kwargs, bool is_keyword, const std::size_t id)
    :BaseASTNode(id), name(n), type(t), is_args(is_args), is_kwargs(is_kwargs), is_keyword(is_keyword) {}

  std::string print(size_t indent_size) override;


};

class FunctionArgumentListASTNode final : public BaseASTNode
{
public:
  std::vector<std::unique_ptr<FunctionArgumentASTNode>> args;
  std::unique_ptr<FunctionArgumentASTNode> args_arg, kwargs_arg;
  explicit FunctionArgumentListASTNode(std::vector<std::unique_ptr<FunctionArgumentASTNode>>& args, const std::size_t id)
    :BaseASTNode(id), args(std::move(args)) {}
  explicit FunctionArgumentListASTNode(std::vector<std::unique_ptr<FunctionArgumentASTNode>>&& args, const std::size_t id)
    :BaseASTNode(id), args(std::move(args)) {}

  explicit FunctionArgumentListASTNode(
    std::vector<std::unique_ptr<FunctionArgumentASTNode>>& args,
    std::unique_ptr<FunctionArgumentASTNode> args_arg,
    std::unique_ptr<FunctionArgumentASTNode> kwargs_arg,
    const std::size_t id
  ):
    BaseASTNode(id),
    args(std::move(args)),
    args_arg(std::move(args_arg)),
    kwargs_arg(std::move(kwargs_arg)) {}

  explicit FunctionArgumentListASTNode(
    std::vector<std::unique_ptr<FunctionArgumentASTNode>>&& args,
    std::unique_ptr<FunctionArgumentASTNode> args_arg,
    std::unique_ptr<FunctionArgumentASTNode> kwargs_arg,
    const std::size_t id
  ):
    BaseASTNode(id),
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

  FunctionDeclASTNode(const Token& n, ASTPtr args, const Token& rt, const std::size_t id)
    :BaseASTNode(id), name(n), args(std::move(args)), return_type(rt) {}

  std::string print(size_t indent_size) override;
};

class GetPropertyASTNode final : public BaseASTNode
{
public:
  ASTPtr base_expr;
	Token name;

	GetPropertyASTNode(ASTPtr base_expr, const Token& nm, const std::size_t id)
		:BaseASTNode(id), base_expr(std::move(base_expr)), name(nm) {}

	std::string print(size_t indent_size) override;
  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class SetPropertyASTNode final : public BaseASTNode
{
public:
  ASTPtr base_expr;
	Token name;
  ASTPtr RHS;

	SetPropertyASTNode(ASTPtr base_expr, const Token& nm, ASTPtr RHS, const std::size_t id)
		:BaseASTNode(id), base_expr(std::move(base_expr)), name(nm), RHS(std::move(RHS)) {}

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
	  const Token& nm,
	  std::vector<ASTPtr>& args,
	  const std::size_t id
	)
		:BaseASTNode(id), base_expr(std::move(base_expr)), name(nm), args(std::move(args)) {}

	std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};
class FunctionCallASTNode final : public BaseASTNode
{
public:
  ASTPtr base;
	std::vector<ASTPtr> args;
	FunctionCallASTNode(ASTPtr base, std::vector<ASTPtr>& args, const std::size_t id)
		:BaseASTNode(id), base(std::move(base)), args(std::move(args)) {}

	std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ReturnStatementASTNode final : public BaseASTNode
{
public:
	ASTPtr expr;
	explicit ReturnStatementASTNode(ASTPtr e, const std::size_t id)
		:BaseASTNode(id), expr(std::move(e)) {}

	std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ScopeASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> nodes;
  explicit ScopeASTNode(const std::size_t id)
    :BaseASTNode(id) {}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class FunctionASTNode final : public BaseASTNode
{
public:
  ASTPtr decl;
  ASTPtr body;

  FunctionASTNode(ASTPtr decl, ASTPtr body, const std::size_t id)
    :BaseASTNode(id), decl(std::move(decl)), body(std::move(body)) {}

  std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class IfElseExpressionASTNode final : public BaseASTNode
{
public:
  ASTPtr condition;
  ASTPtr true_scope;
  ASTPtr false_scope;

  IfElseExpressionASTNode(ASTPtr cond, ASTPtr true_scope, ASTPtr false_scope, const std::size_t id)
    :BaseASTNode(id), condition(std::move(cond)), true_scope(std::move(true_scope)), false_scope(std::move(false_scope)) {}

  std::string print(size_t indent_size) override;


  void visit(Visitor &visitor) override { visitor.visit(*this); };
};

class WhileLoopASTNode final : public BaseASTNode
{
public:
  ASTPtr condition;
  ASTPtr scope;

  WhileLoopASTNode(ASTPtr condition, ASTPtr scope, const std::size_t id)
    :BaseASTNode(id), condition(std::move(condition)), scope(std::move(scope)) {}

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

  ForLoopASTNode(ASTPtr decl, ASTPtr cond, ASTPtr inc, ASTPtr scope, const std::size_t id)
    :BaseASTNode(id), declaration(std::move(decl)), condition(std::move(cond)), increment(std::move(inc)), scope(std::move(scope)){}

  std::string print(size_t indent_size) override;

  void visit(Visitor &visitor) override { visitor.visit(*this); }
};

class ForEachLoopASTNode final : public BaseASTNode
{
public:
  Token identifier;
  ASTPtr iterable_expr;
  ASTPtr scope;

  ForEachLoopASTNode(const Token &identifier, ASTPtr iterable, ASTPtr scope, const std::size_t id)
    :BaseASTNode(id), identifier(identifier), iterable_expr(std::move(iterable)), scope(std::move(scope)) {}

  std::string print(size_t indent_size) override;
};

class KeyParamExpressionASTNode final : public BaseASTNode
{
public:
    Token identifier;
    ASTPtr expression;
    explicit KeyParamExpressionASTNode(const Token &identifier, ASTPtr expr, const std::size_t id)
        :BaseASTNode(id), identifier(identifier), expression(std::move(expr)) {}

    std::string print(size_t indent_size) override;
  
    void visit(Visitor &visitor) override { visitor.visit(*this); }
};
class StarredExpressionASTNode final : public BaseASTNode
{
public:
    ASTPtr expression;
    explicit StarredExpressionASTNode(ASTPtr expr, const std::size_t id)
        :BaseASTNode(id), expression(std::move(expr)) {}

    std::string print(size_t indent_size) override;
};

class RootASTNode final : public BaseASTNode
{
public:
  std::vector<ASTPtr> nodes;
  explicit RootASTNode(const std::size_t id)
    :BaseASTNode(id) {}

  std::string print(size_t indent_size) override;

  void visit(ByteCodeVisitor &visitor) { visitor.visit_RootASTNode(*this); }
};
}

