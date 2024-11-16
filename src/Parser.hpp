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

		// TODO: add check when creating vars for collision with arguments

    if (visitor.interpreter.functions.contains(visitor.interpreter.active_func) && visitor.interpreter.functions[visitor.interpreter.active_func]->args.contains(token.value))
      return visitor.interpreter.functions[visitor.interpreter.active_func]->args[token.value]->value;

    if (visitor.interpreter.functions.contains(visitor.interpreter.active_func) && visitor.interpreter.functions[visitor.interpreter.active_func]->vars.contains(token.value))
      return visitor.interpreter.functions[visitor.interpreter.active_func]->vars[token.value]->value;
		
    if (visitor.interpreter.vars.contains(token.value))
      return visitor.interpreter.vars[token.value]->value;
		
		// std::cout << "active func: " << visitor.interpreter.active_func << " ";  
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
    if (op.type == TOKEN_TYPE::LT)
      return lhs < rhs;
    if (op.type == TOKEN_TYPE::GT)
      return lhs > rhs;
    if (op.type == TOKEN_TYPE::LQ)
      return lhs <= rhs;
    if (op.type == TOKEN_TYPE::GQ)
      return lhs >= rhs;
    if (op.type == TOKEN_TYPE::EQ)
      return lhs == rhs;
		return -1;
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

class FunctionArgumentASTNode : public BaseASTNode
{
public:
  Token name;
  Token type;

  FunctionArgumentASTNode(const Token& n, const Token& t)
    :BaseASTNode(), name(n), type(t) {}


  std::string print() override 
	{ 
		return "arg:(" + print_token(name) + " " + print_token(type) + ")"; 
	};
  int visit(Visitor &visitor) override 
	{
		auto& func = visitor.interpreter.functions[visitor.interpreter.active_func];
		func->args[name.value] = std::make_unique<Variable>(false, 0); // TODO: change
		func->arg_names.push_back(name.value);
		return 0;
	};
};

class FunctionArgumentListASTNode : public BaseASTNode
{
public:
  std::vector<std::unique_ptr<FunctionArgumentASTNode>> args;
  FunctionArgumentListASTNode(std::vector<std::unique_ptr<FunctionArgumentASTNode>>& args)
    :BaseASTNode(), args(std::move(args)) {}

  std::string print() override 
	{ 
		std::string ret = "args:(";
		for (const auto& arg : args)
			 ret += arg->print() + ", ";
		return ret + ")";
	};
  int visit(Visitor &visitor) override 
	{
		for (const auto& arg : args) 
		{
			arg->visit(visitor);
		}
		return 0;
	};
};


class FunctionDeclASTNode : public BaseASTNode
{
public:
  Token name;
  std::unique_ptr<BaseASTNode> args;
  Token return_type;

  FunctionDeclASTNode(const Token& n, std::unique_ptr<BaseASTNode> args, const Token& rt)
    :BaseASTNode(), name(n), args(std::move(args)), return_type(rt) {}

  std::string print() override 
	{ 
		std::string ret = "decl:(name: (" + print_token(name) + "), " + args->print() + ", ret:("+print_token(return_type)+"))";
		return ret;
	};
  int visit(Visitor &visitor) override 
	{
		if (visitor.interpreter.functions.contains(name.value)) 
		{
			std::cout << "Function already exists\n";
			return 0;
		}
		visitor.interpreter.active_func = name.value;
		visitor.interpreter.functions[name.value] = std::make_unique<Function>();
		args->visit(visitor);
		visitor.interpreter.active_func = "";
		return 0;
	};
};

class FunctionCallASTNode : public BaseASTNode 
{
public:
	Token name;
	std::vector<std::unique_ptr<BaseASTNode>> args;
	FunctionCallASTNode(const Token& id, std::vector<std::unique_ptr<BaseASTNode>>& args)
		:BaseASTNode(), name(id), args(std::move(args)) {}

	std::string print() override 
	{ 
		std::string ret = "call:(" + print_token(name) + "(args:(";
		for (const auto& i : args) 
			ret += i->print() + ", ";
		ret += "))";
		return ret;
	}
	int visit(Visitor &visitor) override 
	{
		if (!visitor.interpreter.functions.contains(name.value)) 
		{
			std::cout << "Function doesn't exist\n";
			return 0;
		}

		// set args with passed expressions
		for (size_t i = 0; i < args.size(); i++) 
		{
			auto& f_arg = visitor.interpreter.functions[name.value]->args[visitor.interpreter.functions[name.value]->arg_names[i]];
			f_arg->value = args[i]->visit(visitor);
		}
	  //set defaults
	  visitor.interpreter.functions[name.value]->should_return = false;
	  visitor.interpreter.functions[name.value]->return_value = -1;

		visitor.interpreter.active_func = name.value;
		auto v = visitor.interpreter.functions[name.value]->ast->visit(visitor);
		visitor.interpreter.active_func = "";
		return v;
	}
};

class ReturnStatementASTNode : public BaseASTNode 
{
public:
	std::unique_ptr<BaseASTNode> expr;
	ReturnStatementASTNode(std::unique_ptr<BaseASTNode> e)
		:expr(std::move(e)) {}

	std::string print() override { return "return:(" + expr->print() + ")"; }
	int visit(Visitor &visitor) override
	{
	  auto var = expr->visit(visitor);
    if (!visitor.interpreter.active_func.empty())
    {
      auto& func = visitor.interpreter.functions[visitor.interpreter.active_func];
      func->should_return = true;
      func->return_value = var;
    }
	  return var;
	}
};

// fn penis(a: uint64, b: str): str {
//    return b * a;
// }
class ScopeASTNode : public BaseASTNode
{
public:
  std::vector<std::unique_ptr<BaseASTNode>> nodes;
  ScopeASTNode()
    :BaseASTNode() {}

  std::string print() override 
	{ 
    std::string res{"scope:(\n"};
    for (const auto& i : nodes)
    {
      res += " " + i->print() + ";\n";
    }
    return res + ")";
	};
  int visit(Visitor &visitor) override 
	{
		for (const auto& i : nodes) {
		  if (!visitor.interpreter.active_func.empty())
		  {
        auto& func = visitor.interpreter.functions[visitor.interpreter.active_func];
		    if (func->should_return)
		      return func->return_value;
		  }

			// return after we hit first return statement;
			if (dynamic_cast<ReturnStatementASTNode*>(i.get()) != nullptr) {
				return i->visit(visitor);
			}
			i->visit(visitor);
			// std::cout << i->visit(visitor) << " ";
		}

    if (!visitor.interpreter.active_func.empty())
    {
      auto& func = visitor.interpreter.functions[visitor.interpreter.active_func];
      if (func->should_return)
        return func->return_value;
    }
		return 0;
	};
};

class FunctionASTNode : public BaseASTNode
{
public:
  std::unique_ptr<BaseASTNode> decl;
  std::unique_ptr<BaseASTNode> body;

  FunctionASTNode(std::unique_ptr<BaseASTNode> decl, std::unique_ptr<BaseASTNode> body)
    :BaseASTNode(), decl(std::move(decl)), body(std::move(body)) {}

  std::string print() override 
	{ 
		return "function:(" + decl->print() + " " + body->print() + ")"; 
	};
  int visit(Visitor &visitor) override 
	{
		decl->visit(visitor);
		visitor.interpreter.functions[((FunctionDeclASTNode*)decl.get())->name.value]->ast = std::move(body); // THIS IS PROBABLY WRONG!!!!!!!!!!!!!1
		return 0;
	};
};

class IfElseExpressionASTNode : public BaseASTNode
{
public:
  std::unique_ptr<BaseASTNode> condition;
  std::unique_ptr<BaseASTNode> true_scope;
  std::unique_ptr<BaseASTNode> false_scope;

  IfElseExpressionASTNode(std::unique_ptr<BaseASTNode> cond, std::unique_ptr<BaseASTNode> ts, std::unique_ptr<BaseASTNode> fs = nullptr)
    :BaseASTNode(), condition(std::move(cond)), true_scope(std::move(ts)), false_scope(std::move(fs)) {}

  std::string print() override
  {
    return "if:(" + condition->print() + " " + true_scope->print() + ")";
  }
  int visit(Visitor &visitor) override
  {
    if (condition->visit(visitor) == true)
      return true_scope->visit(visitor);
    return false_scope->visit(visitor);
  }
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

  std::unique_ptr<BaseASTNode> parse_literal();
  std::unique_ptr<BaseASTNode> parse_identifier();
  std::unique_ptr<BaseASTNode> parse_primary_expr();
  std::unique_ptr<BaseASTNode> parse_unary();
  std::unique_ptr<BaseASTNode> parse_paren_expr();
  std::unique_ptr<BaseASTNode> parse_expr();
	std::unique_ptr<BaseASTNode> parse_return();
  std::unique_ptr<BaseASTNode> parse_binop_rhs(int expr_prec, std::unique_ptr<BaseASTNode> LHS);
  std::unique_ptr<BaseASTNode> parse_var_decl();
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

