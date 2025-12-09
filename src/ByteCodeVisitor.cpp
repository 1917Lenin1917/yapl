//
// Created by Максим Литвиненко on 27.08.2025.
//

#include "yapl/ByteCodeVisitor.hpp"
#include "yapl/values/CodeObjectValue.hpp"
#include "yapl/values/IntegerValue.hpp"

#include "yapl/ASTNode.hpp"

namespace yapl {

// 1 + 2
// LOAD_CONST 0
// LOAD_CONST 1
// BIN_OP 0

// 1. visit_BinaryOp (1 + 2)
// 1.1. visit_Literal (1)
// 1.1.1. check if 1 exists in constants, if not, push it
// 1.1.2. add LOAD_CONST {idx} to opcode of current object
// 1.2. visit_Literal (2)
// 1.2.1. check if 2 exists in constants, if not, push it
// 1.2.2. add LOAD_CONST {idx} to opcode of current object
// 1.2. add BIN_OP 0 to opcode of current object

CodeObject ByteCodeVisitor::visit_RootASTNode(const RootASTNode &node)
{
  m_ObjectStack.push_back({});
  m_ScopeVars.push_back({});

  for (const auto& child_node : node.nodes)
  {
    child_node->visit(*this);
  }
  m_ObjectStack.back().OpCodes.push_back(HALT);
  m_ScopeVars.pop_back();

  return m_ObjectStack.back();

}

void ByteCodeVisitor::visit_VariableASTNode(const VariableASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  const auto var_name = std::string(node.name.value);
  std::size_t idx = current_object.LocalsMap.contains(var_name) ? current_object.LocalsMap.at(var_name) : -1;
  if (idx == -1)
  {
    auto is_const = node.type.type == TOKEN_TYPE::CONST;
    const auto var = std::make_shared<Variable>(is_const, VALUE_TYPE::UNDEFINED, nullptr, "TODO", node.name.value);
    current_object.Locals.push_back(var);
    current_object.Names.emplace_back(node.name.value);
    idx = current_object.Locals.size() - 1;
    current_object.LocalsMap[var_name] = idx;
  }

  if (node.value) { node.value->visit(*this); }
  else { current_object.OpCodes.push_back(LOAD_UNDEF); }

  current_object.OpCodes.push_back(INIT_VAR);
  current_object.OpCodes.push_back(static_cast<OpCode>(idx));

  m_ScopeVars.back().push_back(idx);
}

void ByteCodeVisitor::visit_BinaryOpASTNode(const BinaryOpASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  node.LHS->visit(*this);
  node.RHS->visit(*this);

  current_object.OpCodes.push_back(BINARY_OP);

  switch (node.op.type) {
    case TOKEN_TYPE::PLUS:
      current_object.OpCodes.push_back(static_cast<OpCode>(ADD));
      break;

    case TOKEN_TYPE::TIMES:
      current_object.OpCodes.push_back(static_cast<OpCode>(MUL));
      break;

    case TOKEN_TYPE::MINUS:
      current_object.OpCodes.push_back(static_cast<OpCode>(SUB));
      break;

    case TOKEN_TYPE::SLASH:
      current_object.OpCodes.push_back(static_cast<OpCode>(DIV));
      break;

    case TOKEN_TYPE::MOD:
      current_object.OpCodes.push_back(static_cast<OpCode>(MOD));
      break;

    case TOKEN_TYPE::LT:
      current_object.OpCodes.push_back(static_cast<OpCode>(LT));
      break;

    case TOKEN_TYPE::GT:
      current_object.OpCodes.push_back(static_cast<OpCode>(GT));
      break;

    case TOKEN_TYPE::LQ:
      current_object.OpCodes.push_back(static_cast<OpCode>(LQ));
      break;

    case TOKEN_TYPE::GQ:
      current_object.OpCodes.push_back(static_cast<OpCode>(GQ));
      break;

    case TOKEN_TYPE::EQ:
      current_object.OpCodes.push_back(static_cast<OpCode>(EQ));
      break;

    case TOKEN_TYPE::NEQ:
      current_object.OpCodes.push_back(static_cast<OpCode>(NEQ));
      break;

    case TOKEN_TYPE::AND:
      current_object.OpCodes.push_back(static_cast<OpCode>(AND));
      break;

    case TOKEN_TYPE::OR:
        current_object.OpCodes.push_back(static_cast<OpCode>(OR));
        break;

    default: throw std::runtime_error("Unhandled dtoken");
  }

}

void ByteCodeVisitor::visit_IdentifierASTNode(const IdentifierASTNode &node)
{
  // TODO: throw compilation error if variable doesnt exist


  auto& current_object = m_ObjectStack.back();

  std::string name = node.token.value;
  auto idx = current_object.NamesMap.contains(name) ? current_object.NamesMap.at(name) : -1;

  if (idx == -1)
  {
    current_object.Names.push_back(name);
    idx = current_object.Names.size() - 1;
    current_object.NamesMap[name] = idx;
  }

  if (next_identifier_as_store_name)
  {
    current_object.OpCodes.push_back(STORE_NAME);
    current_object.OpCodes.push_back(static_cast<OpCode>(idx));
    next_identifier_as_store_name = false;
    return;
  }

  current_object.OpCodes.push_back(LOAD_NAME);
  current_object.OpCodes.push_back(static_cast<OpCode>(idx));
}

std::size_t ByteCodeVisitor::visit_IntegerASTNode(const IntegerASTNode &node)
{
  auto& current_object = m_ObjectStack.back();
  const auto key = std::to_string(node.value);

  if (!current_object.ConstantsMap.contains(key))
  {
    current_object.Constants.push_back(mk_int(node.value));
    const auto len = current_object.Constants.size();
    current_object.ConstantsMap[key] = len - 1;

    current_object.OpCodes.push_back(LOAD_CONST);
    current_object.OpCodes.push_back(static_cast<OpCode>(len - 1));

    return len - 1;
  }
  auto idx = current_object.ConstantsMap.at(key);

  current_object.OpCodes.push_back(LOAD_CONST);
  current_object.OpCodes.push_back(static_cast<OpCode>(idx));

  return idx;
}

void ByteCodeVisitor::visit_FloatASTNode(const FloatASTNode &node)
{
}

void ByteCodeVisitor::visit_BooleanASTNode(const BooleanASTNode &node)
{
}

void ByteCodeVisitor::visit_StringASTNode(const StringASTNode &node)
{
}

void ByteCodeVisitor::visit_IfElseExpressionASTNode(const IfElseExpressionASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  node.condition->visit(*this);
  current_object.OpCodes.push_back(JMP_IF_FALSE);
  // temp push 0, and save index. after we compute if branch, we go back and update jump amount
  current_object.OpCodes.push_back(static_cast<OpCode>(0));
  auto jmp_len_idx = current_object.OpCodes.size() - 1;

  node.true_scope->visit(*this);
  // if there is an else branch, we should jump over it

  if (node.false_scope)
  {
    // temp push 0, and save index. after we compute else branch, we go back and update jump amount
    current_object.OpCodes.push_back(JMP);
    current_object.OpCodes.push_back(static_cast<OpCode>(0));
    auto jmp_len_idx_2 = current_object.OpCodes.size() - 1;
    current_object.OpCodes[jmp_len_idx] = static_cast<OpCode>(current_object.OpCodes.size() - jmp_len_idx - 1);

    node.false_scope->visit(*this);
    current_object.OpCodes[jmp_len_idx_2] = static_cast<OpCode>(current_object.OpCodes.size() - jmp_len_idx_2 - 1);
  }
}

void ByteCodeVisitor::visit_ScopeASTNode(const ScopeASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  m_ScopeVars.emplace_back();
  for (const auto& child_node : node.nodes)
  {
    child_node->visit(*this);
  }
  for (std::size_t idx : m_ScopeVars.back())
  {
    current_object.OpCodes.push_back(DEINIT_VAR);
    current_object.OpCodes.push_back(static_cast<OpCode>(idx));
  }
  m_ScopeVars.pop_back();
}

void ByteCodeVisitor::visit_ForLoopASTNode(const ForLoopASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  // 1) init: runs once
  if (node.declaration) {
    node.declaration->visit(*this);
  }

  // 2) mark the start of the condition
  const size_t cond_start = current_object.OpCodes.size();

  // 3) condition
  if (node.condition) {
    node.condition->visit(*this);
  }
  current_object.OpCodes.push_back(JMP_IF_FALSE);
  current_object.OpCodes.push_back(static_cast<OpCode>(0)); // placeholder
  const size_t jmp_out_len_idx = current_object.OpCodes.size() - 1; // index of placeholder

  // 4) body
  if (node.scope) {
    node.scope->visit(*this);
  }

  // 5) increment (runs after body each iteration)
  if (node.increment) {
    node.increment->visit(*this);
  }

  // 6) jump back to condition start (backward jump)
  current_object.OpCodes.push_back(JMP);
  current_object.OpCodes.push_back(static_cast<OpCode>(0)); // placeholder
  const size_t jmp_back_len_idx = current_object.OpCodes.size() - 1;

  // Patch backward jump:
  // immediate = target_index - (len_idx + 1)  ==> jump from after-immediate to cond_start
  {
    const int back = static_cast<int>(cond_start) - static_cast<int>(jmp_back_len_idx + 1);
    current_object.OpCodes[jmp_back_len_idx] = static_cast<OpCode>(back);
  }

  // Patch "exit the loop" forward jump:
  // immediate = end_index - (len_idx + 1)  ==> jump from after-immediate to after-loop
  {
    const int out = static_cast<int>(current_object.OpCodes.size()) - static_cast<int>(jmp_out_len_idx + 1);
    current_object.OpCodes[jmp_out_len_idx] = static_cast<OpCode>(out);
  }
}

void ByteCodeVisitor::visit_WhileLoopASTNode(const WhileLoopASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  // 1) mark the start of the condition
  const size_t cond_start = current_object.OpCodes.size();

  // 2) condition
  if (node.condition) {
    node.condition->visit(*this); // leaves truthy/falsy on stack
  }
  // If false -> jump to end (placeholder now, patch later)
  current_object.OpCodes.push_back(JMP_IF_FALSE);
  current_object.OpCodes.push_back(static_cast<OpCode>(0));
  const size_t jmp_out_len_idx = current_object.OpCodes.size() - 1;

  // 3) body
  if (node.scope) {
    node.scope->visit(*this);
  }

  // 4) jump back to condition start
  current_object.OpCodes.push_back(JMP);
  current_object.OpCodes.push_back(static_cast<OpCode>(0)); // placeholder
  const size_t jmp_back_len_idx = current_object.OpCodes.size() - 1;

  // --- patching ---

  // Backward jump: from after-immediate to cond_start
  {
    const int back = static_cast<int>(cond_start) - static_cast<int>(jmp_back_len_idx + 1);
    current_object.OpCodes[jmp_back_len_idx] = static_cast<OpCode>(back);
  }

  // Exit jump: from after-immediate to after-loop (current end)
  {
    const int out = static_cast<int>(current_object.OpCodes.size()) - static_cast<int>(jmp_out_len_idx + 1);
    current_object.OpCodes[jmp_out_len_idx] = static_cast<OpCode>(out);
  }
}

void ByteCodeVisitor::visit_StatementASTNode(const StatementASTNode &node)
{
  // for now just handle assignment to variable statement
  // TODO: handle method and property assignment

  // push rhs to stack
  node.RHS->visit(*this);

  next_identifier_as_store_name = true;

  node.base->visit(*this);

}

void ByteCodeVisitor::visit_FunctionASTNode(const FunctionASTNode &node)
{
  // push arguments to locals
  // 1. generate a new code object for the function and push it in constants pool

  m_ObjectStack.emplace_back();

  const auto decl = static_cast<FunctionDeclASTNode*>(node.decl.get());
  const std::string name = decl->name.value;
  decl->args->visit(*this);

  node.body->visit(*this);

  auto f_code_object = std::move(m_ObjectStack.back());
  m_ObjectStack.pop_back();

  auto& current_object = m_ObjectStack.back();


  auto ptr = std::make_shared<CodeObject>(std::move(f_code_object));
  current_object.Constants.push_back(std::make_shared<CodeObjectValue>(ptr));
  auto idx = current_object.Constants.size() - 1;

  current_object.ConstantsMap[name] = idx;

  const auto var = std::make_shared<Variable>(true, VALUE_TYPE::UNDEFINED, nullptr, "TODO", name);
  current_object.Locals.push_back(var);
  auto locals_idx = current_object.Locals.size() - 1;
  current_object.LocalsMap[name] = locals_idx;

  // 2. generate opcode to load the code object from constants and generate a function object
  current_object.OpCodes.push_back(LOAD_CONST);
  current_object.OpCodes.push_back(static_cast<OpCode>(idx));

  current_object.OpCodes.push_back(MAKE_FUNC);

  current_object.OpCodes.push_back(STORE_NAME);
  current_object.OpCodes.push_back(static_cast<OpCode>(locals_idx));
}

void ByteCodeVisitor::visit_ReturnStatementASTNode(const ReturnStatementASTNode &node)
{
  node.expr->visit(*this);

  auto& current_object = m_ObjectStack.back();
  current_object.OpCodes.push_back(RETURN);
}

void ByteCodeVisitor::visit_FunctionCallASTNode(const FunctionCallASTNode &node)
{
  for (const auto& arg : node.args) arg->visit(*this);

  node.base->visit(*this);

  auto& current_object = m_ObjectStack.back();
  current_object.OpCodes.push_back(CALL);
  current_object.OpCodes.push_back(static_cast<OpCode>(node.args.size()));
}

void ByteCodeVisitor::visit_FunctionArgumentListASTNode(const FunctionArgumentListASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  for (const auto& arg : node.args)
  {
    const auto var_name = std::string(arg->name.value);
    std::size_t idx = current_object.LocalsMap.contains(var_name) ? current_object.LocalsMap.at(var_name) : -1;
    if (idx == -1)
    {
      const auto var = std::make_shared<Variable>(false, VALUE_TYPE::UNDEFINED, nullptr, "TODO", var_name);
      current_object.Locals.push_back(var);
      current_object.Names.emplace_back(var_name);
      idx = current_object.Locals.size() - 1;
      current_object.LocalsMap[var_name] = idx;
    }
    m_ScopeVars.back().push_back(idx);
  }
}
}
