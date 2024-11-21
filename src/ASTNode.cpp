//
// Created by lenin on 17.11.2024.
//

#include "ASTNode.hpp"

#include <algorithm>

namespace yapl {

//
// LiteralASTNode
//
std::string LiteralASTNode::print()
{
  return "literal:(" + std::string(token.value) + ")";
}

std::shared_ptr<Value> LiteralASTNode::visit(Visitor &visitor)
{
  switch (token.type)
  {
    case TOKEN_TYPE::INTEGER: return std::make_unique<IntegerValue>(std::stoi(token.value));
    case TOKEN_TYPE::BOOL: return std::make_unique<BooleanValue>(token.value == std::string("true"));
    case TOKEN_TYPE::FLOAT: return std::make_unique<FloatValue>(std::stof(token.value));
    case TOKEN_TYPE::STRING: return std::make_unique<StringValue>(std::string(token.value));
    default:
    {
      std::cerr << "Unknown literal type!\n";
      return nullptr;
    }
  }
}


//
// IdentifierASTNode
//
std::string IdentifierASTNode::print()
{
  return "identifier:(" + std::string(token.value) + ")";
}

std::shared_ptr<Value> IdentifierASTNode::visit(Visitor &visitor)
{
  for (int i = visitor.interpreter.scope_stack.size() - 1; i >= 0; i--)
  {
    const auto& scope = visitor.interpreter.scope_stack[i];
    if (scope->vars.contains(token.value))
    {
      auto original_value = scope->vars[token.value]->value;
      // auto ret_value = original_value->Copy();
      // return ret_value;
      return original_value;
    }
  }
  std::cerr << std::format("Variable {} doesn't exist.\n", token.value);
  return nullptr;
}



//
// IndexASTNode
//
std::string IndexASTNode::print()
{
  return "index: (" + base_expr->print() + " [" + index_expr->print() +"])";
}

std::shared_ptr<Value> IndexASTNode::visit(Visitor &visitor)
{
  auto v = base_expr->visit(visitor)->OperatorIndex(index_expr->visit(visitor));
  return std::move(v);
}

//
// ArrayASTNode
//
std::string ArrayASTNode::print()
{
  std::string ret_value = "array: (";
  for (const auto& v : values)
    ret_value += v->print() + ", ";
  return ret_value + ")";
}

std::shared_ptr<Value> ArrayASTNode::visit(Visitor &visitor)
{
  std::vector<std::shared_ptr<Value>> ev_values;
  for (const auto& value : values)
  {
    ev_values.push_back(value->visit(visitor));
  }
  return std::make_shared<ArrayValue>(ev_values);
}


//
// VariableASTNode
//
std::string VariableASTNode::print()
{
  if (value == nullptr)
    return "variable:(" + print_token(type) + " " + name.value + ")";
  return "variable:(" + print_token(type) + " " + name.value + " = " + value->print() + ")";
}

std::shared_ptr<Value> VariableASTNode::visit(Visitor &visitor)
{
  for (size_t i = visitor.interpreter.scope_stack.size(); i --> 0;)
  {
    const auto& scope = visitor.interpreter.scope_stack[i];
    if (scope->vars.contains(name.value))
    {
      std::cerr << std::format("Variable {} is already defined exist.\n", name.value);
      return nullptr;
    }
  }

  auto scope = visitor.interpreter.scope_stack[visitor.interpreter.scope_stack.size() - 1];
  if (value)
  {
    auto expr_value = value->visit(visitor);
    scope->vars[name.value] = std::make_shared<Variable>(type.type == TOKEN_TYPE::CONST, VALUE_TYPE::INTEGER, expr_value);
    return nullptr;
  }
  scope->vars[name.value] = std::make_shared<Variable>(type.type == TOKEN_TYPE::CONST, VALUE_TYPE::INTEGER, std::make_unique<IntegerValue>(0));
  return nullptr;
}


//
// UnaryOpASTNode
//
std::string UnaryOpASTNode::print()
{
  return "unary:(" + print_token(op) + " " + RHS->print() + ")";
}

std::shared_ptr<Value> UnaryOpASTNode::visit(Visitor &visitor)
{
  switch (op.type)
  {
    case TOKEN_TYPE::MINUS: { return RHS->visit(visitor)->UnaryMinus(); }
    case TOKEN_TYPE::PLUS: { return RHS->visit(visitor)->UnaryPlus(); }
    case TOKEN_TYPE::NOT: { return RHS->visit(visitor)->UnaryNot(); }
    default: { std::cerr << "Unary unhandled default\n"; return nullptr; }
  }
}


//
// BinaryOpASTNode
//
std::string BinaryOpASTNode::print()
{
  return "binary:(" + LHS->print() + " " + print_token(op) + " " + RHS->print() + ")";
}

std::shared_ptr<Value> BinaryOpASTNode::visit(Visitor &visitor)
{
  const auto lhs = LHS->visit(visitor);
  const auto rhs = RHS->visit(visitor);
  if (op.type == TOKEN_TYPE::PLUS)
    return lhs->BinaryPlus(rhs);
  if (op.type == TOKEN_TYPE::TIMES)
    return lhs->BinaryTimes(rhs);
  if (op.type == TOKEN_TYPE::MINUS)
    return lhs->BinaryMinus(rhs);
  if (op.type == TOKEN_TYPE::SLASH)
    return lhs->BinarySlash(rhs);
  if (op.type == TOKEN_TYPE::LT)
    return lhs->BinaryLT(rhs);
  if (op.type == TOKEN_TYPE::GT)
    return lhs->BinaryGT(rhs);
  if (op.type == TOKEN_TYPE::LQ)
    return lhs->BinaryLQ(rhs);
  if (op.type == TOKEN_TYPE::GQ)
    return lhs->BinaryGQ(rhs);
  if (op.type == TOKEN_TYPE::EQ)
    return lhs->BinaryEQ(rhs);
  return nullptr;
}


//
// StatementASTNode
//
std::string StatementASTNode::print()
{
  return "stmnt:(" + print_token(identifier) + " " + RHS->print() + ")";
}

std::shared_ptr<Value> StatementASTNode::visit(Visitor &visitor)
{
  for (size_t i = visitor.interpreter.scope_stack.size(); i --> 0 ;)
  {
    const auto& scope = visitor.interpreter.scope_stack[i];
    if (scope->vars.contains(identifier.value))
    {
      auto& var = scope->vars[identifier.value];
      if (var->is_const)
      {
        std::cerr << std::format("Variable {} is constant.\n", identifier.value);
        return nullptr;
      }
      auto rhs = RHS->visit(visitor);
      var->value = std::move(rhs); // does this get freed automatically?
      return nullptr;
    }
  }

  std::cerr << std::format("Variable {} doesn't exist.\n", identifier.value);
  return nullptr;
}

//
// StatementIndexASTNode
//
std::string StatementIndexASTNode::print()
{
  return "stmnt:(" + identifier->print() + " " + RHS->print() + ")";
}

std::shared_ptr<Value> StatementIndexASTNode::visit(Visitor &visitor)
{
  auto id = dynamic_cast<IndexASTNode*>(identifier.get());
  auto tk = dynamic_cast<IdentifierASTNode*>(id->base_expr.get());

  auto lhs = id->base_expr->visit(visitor);
  auto idx = id->index_expr->visit(visitor);
  auto rhs = RHS->visit(visitor);
  lhs->OperatorIndexSet(idx, rhs);
  return nullptr;
  //
  // for (int i = visitor.interpreter.scope_stack.size() - 1; i >= 0; i--)
  // {
  //   const auto& scope = visitor.interpreter.scope_stack[i];
  //   if (scope->vars.contains(tk->token.value))
  //   {
  //     auto& original_var = scope->vars[tk->token.value];
  //     original_var->value->OperatorIndexSet(id->index_expr->visit(visitor), RHS->visit(visitor));
  //     // original_var->value->Set(RHS->visit(visitor));
  //     return nullptr;
  //   }
  // }
  // std::cerr << "No such variable\n";
  // return nullptr;
}

//
// FunctionArgumentASTNode
//
std::string FunctionArgumentASTNode::print()
{
  return "arg:(" + print_token(name) + " " + print_token(type) + ")";
}

std::shared_ptr<Value> FunctionArgumentASTNode::visit(Visitor &visitor)
{
  return nullptr;
  // auto& func = visitor.interpreter.functions[visitor.interpreter.active_func];
  // func->args[name.value] = std::make_unique<Variable>(false, 0); // TODO: change
  // func->arg_names.push_back(name.value);
  // return 0;
}


//
// FunctionArgumentListASTNode
//
std::string FunctionArgumentListASTNode::print()
{
  std::string ret = "args:(";
  for (const auto& arg : args)
     ret += arg->print() + ", ";
  return ret + ")";
}

std::shared_ptr<Value> FunctionArgumentListASTNode::visit(Visitor &visitor)
{
  return nullptr;
  // for (const auto& arg : args)
  // {
  //   arg->visit(visitor);
  // }
  // return 0;
}


//
// FunctionDeclASTNode
//
std::string FunctionDeclASTNode::print()
{
  std::string ret = "decl:(name: (" + print_token(name) + "), " + args->print() + ", ret:("+print_token(return_type)+"))";
  return ret;
}

std::shared_ptr<Value> FunctionDeclASTNode::visit(Visitor &visitor)
{
  return nullptr;
}


//
// MethodCallASTNode
//
std::string MethodCallASTNode::print()
{
  std::string ret = "method:(" + print_token(name) + "(args:(";
  for (const auto& i : args)
    ret += i->print() + ", ";
  ret += "))";
  return ret;
}

std::shared_ptr<Value> MethodCallASTNode::visit(Visitor &visitor)
{
  auto object = visitor.interpreter.get_variable(identifier.value);
  if (!object)
  {
    std::cerr << "Object " << identifier.value << " doesn't exist\n";
    return nullptr;
  }

  auto func_def = object->value->get_method_definition(name.value);
  auto func = visitor.interpreter.push_function(std::string(identifier.value) + "." + name.value); // maybe get this from value ?
  auto arg_list = dynamic_cast<FunctionArgumentListASTNode*>(dynamic_cast<FunctionDeclASTNode*>(func_def->decl.get())->args.get());
  size_t arg_amount = arg_list->get_argument_amount();
  if (arg_amount != -1 && args.size() != arg_amount)
  {
    std::cerr << std::format("Invalid amount of arguments provided to function {}. Expected {} arguments, instead got {}.\n", name.value, arg_amount, args.size());
    return nullptr;
  }
  // set args with passed expressions
  for (size_t i = 0; i < args.size(); i++)
  {
    auto value = args[i]->visit(visitor);
    func->set_argument(std::make_unique<Variable>(false, value->type, std::move(value)), arg_list->get_argument_name(i));
    // func->function_scope->vars[arg_list->args[i].get()->name.value] = std::make_unique<Variable>(false, value->type, std::move(value));
  }
  func_def->body->visit(visitor);
  auto ret_value = func->return_value;
  visitor.interpreter.pop_scope();
  visitor.interpreter.pop_function();

  return ret_value;
}



//
// FunctionCallASTNode
//
std::string FunctionCallASTNode::print()
{
  std::string ret = "call:(" + print_token(name) + "(args:(";
  for (const auto& i : args)
    ret += i->print() + ", ";
  ret += "))";
  return ret;
}

std::shared_ptr<Value> FunctionCallASTNode::visit(Visitor &visitor)
{
  if (!visitor.interpreter.function_exists(name.value))
  {
    std::cout << "Function doesn't exist\n";
    return nullptr;
  }

  auto func_def = visitor.interpreter.get_function_def(name.value);
  auto func = visitor.interpreter.push_function(name.value);
  auto arg_list = dynamic_cast<FunctionArgumentListASTNode*>(dynamic_cast<FunctionDeclASTNode*>(func_def->decl.get())->args.get());
  size_t arg_amount = arg_list->get_argument_amount();
  if (arg_amount != -1 && args.size() != arg_amount)
  {
    std::cerr << std::format("Invalid amount of arguments provided to function {}. Expected {} arguments, instead got {}.\n", name.value, arg_amount, args.size());
    return nullptr;
  }
  // set args with passed expressions
  for (size_t i = 0; i < args.size(); i++)
  {
    auto value = args[i]->visit(visitor);
    func->set_argument(std::make_unique<Variable>(false, value->type, std::move(value)), arg_list->get_argument_name(i));
    // func->function_scope->vars[arg_list->args[i].get()->name.value] = std::make_unique<Variable>(false, value->type, std::move(value));
  }
  func_def->body->visit(visitor);
  auto ret_value = func->return_value;
  visitor.interpreter.pop_scope();
  visitor.interpreter.pop_function();

  return ret_value;
}


//
// ReturnStatementASTNode
//
std::string ReturnStatementASTNode::print()
{
  return "return:(" + expr->print() + ")";
}

std::shared_ptr<Value> ReturnStatementASTNode::visit(Visitor &visitor)
{
  auto func = visitor.interpreter.function_stack[visitor.interpreter.function_stack.size() - 1];
  auto rhs = expr->visit(visitor);
  func->return_value = rhs;
  return nullptr;
}


//
// ScopeASTNode
//
// TODO: maybe handle scope creation here?
std::string ScopeASTNode::print()
{
  std::string res{"scope:(\n"};
  for (const auto& i : nodes)
  {
    if (i != nullptr)
    {
      res += " " + i->print() + ";\n";
    }
    else
    {
      res += "node is a nullptr\n";
    }
  }
  return res + ")";
}

std::shared_ptr<Value> ScopeASTNode::visit(Visitor &visitor)
{
  const auto current_func = visitor.interpreter.function_stack.empty() ? nullptr : visitor.interpreter.function_stack[visitor.interpreter.function_stack.size() - 1];
  for (const auto& i : nodes)
  {
    i->visit(visitor);

    if (current_func != nullptr && current_func->return_value != nullptr)
      return nullptr;
    //   return std::move(current_func->return_value);
  }
  return nullptr;
}


//
// FunctionASTNode
//
std::string FunctionASTNode::print()
{
  return "function:(" + decl->print() + " " + body->print() + ")";
}

std::shared_ptr<Value> FunctionASTNode::visit(Visitor &visitor)
{
  const auto& f_decl = dynamic_cast<FunctionDeclASTNode*>(decl.get());
  if (visitor.interpreter.function_exists(f_decl->name.value))
  {
    std::cout << "Function " << f_decl->name.value << " already exists!\n";
    return nullptr;
  }

  visitor.interpreter.add_function_definition(f_decl->name.value, this);

  return nullptr; // should there be a function type??
}



//
// BuiltinCustomVisitFunctionASTNode
//
std::string BuiltinCustomVisitFunctionASTNode::print()
{
  return "";
}

std::shared_ptr<Value> BuiltinCustomVisitFunctionASTNode::visit(Visitor &visitor)
{
  auto func_obj = visitor.interpreter.function_stack[visitor.interpreter.function_stack.size() - 1];
  func_obj->return_value = func(func_obj);
  return nullptr;
}



//
// BuiltinPrintFunctionBodyASTNode
//
std::string BuiltinPrintFunctionBodyASTNode::print()
{
  return "";
}

std::shared_ptr<Value> BuiltinPrintFunctionBodyASTNode::visit(Visitor &visitor)
{
  const auto& func = visitor.interpreter.function_stack[visitor.interpreter.function_stack.size() - 1];
  for (const auto& name : func->argument_names)
  {
    const auto& arg = func->function_scope->vars.at(name);
    std::cout << arg->value->print() << " ";
  }
  std::cout << "\n";
  return nullptr;
}


//
// BuiltinReadIntFunctionBodyASTNode
//
std::string BuiltinReadIntFunctionBodyASTNode::print()
{
  return "";
}

std::shared_ptr<Value> BuiltinReadIntFunctionBodyASTNode::visit(Visitor &visitor)
{
  const auto& func = visitor.interpreter.function_stack[visitor.interpreter.function_stack.size() - 1];
  int value;
  std::cin >> value;
  func->return_value = std::make_unique<IntegerValue>(value);

  return nullptr;
}


//
// BuiltinReadIntFunctionBodyASTNode
//
std::string BuiltinReadStringFunctionBodyASTNode::print()
{
  return "";
}

std::shared_ptr<Value> BuiltinReadStringFunctionBodyASTNode::visit(Visitor &visitor)
{
  const auto& func = visitor.interpreter.function_stack[visitor.interpreter.function_stack.size() - 1];
  std::string value;
  std::cin >> value;
  func->return_value = std::make_unique<StringValue>(value);

  return nullptr;
}


//
// IfElseExpressionASTNode
//
std::string IfElseExpressionASTNode::print()
{
  return "if:(" + condition->print() + " " + true_scope->print() + ")";
}

std::shared_ptr<Value> IfElseExpressionASTNode::visit(Visitor &visitor)
{
  if (dynamic_cast<BooleanValue*>(condition->visit(visitor)->BinaryEQ(std::make_unique<BooleanValue>(true)).get())->value)
    return true_scope->visit(visitor);
  if (false_scope)
    return false_scope->visit(visitor);
  return nullptr;
}



//
// WhileLoopASTNode
//

std::string WhileLoopASTNode::print()
{
  return "";
}

std::shared_ptr<Value> WhileLoopASTNode::visit(Visitor &visitor)
{
  while (dynamic_cast<BooleanValue*>(condition->visit(visitor)->BinaryEQ(std::make_unique<BooleanValue>(true)).get())->value == true)
  {
    scope->visit(visitor);
  }
  return nullptr;
}

//
// RootASTNode
//
std::string RootASTNode::print()
{
  std::string res{"root:(\n"};
  for (const auto& i : nodes)
  {
    res += " " + i->print() + ";\n";
  }
  return res + ")";
}

std::shared_ptr<Value> RootASTNode::visit(Visitor &visitor)
{
  for (const auto& node : nodes)
  {
    auto res = node->visit(visitor);
    if (res != nullptr)
    {
      if (res->type == VALUE_TYPE::INTEGER)
        std::cout << dynamic_cast<IntegerValue*>(res.get())->value << std::endl;
    }
  }
  return nullptr;
}

}
