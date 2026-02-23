//
// Created by Максим Литвиненко on 27.08.2025.
//

#include <ranges>

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
  m_ObjectStack.push_back({ .name = "__main__" });
  m_ScopeVars.push_back({});

  for (const auto& child_node : node.nodes)
  {
    child_node->visit(*this);
  }
  m_ObjectStack.back().op_codes.push_back(HALT);
  m_ScopeVars.pop_back();

  return m_ObjectStack.back();

}

/* Variable declaration node
 *
 * TODO: handle all redeclaration checks, etc.
 */
void ByteCodeVisitor::visit_VariableASTNode(const VariableASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  const auto var_name = std::string(node.name.value);
  const auto it = std::ranges::find(current_object.locals, var_name);

  std::size_t index = it != current_object.locals.end() ? it - current_object.locals.begin() : -1;
  if (index == -1)
  {
    current_object.locals.push_back(var_name);
    index = current_object.locals.size() - 1;
  }

  if (node.value) { node.value->visit(*this); }
  else { current_object.op_codes.push_back(LOAD_UNDEF); }

  current_object.op_codes.push_back(INIT_VAR);
  current_object.op_codes.push_back(static_cast<OpCode>(index));

  m_ScopeVars.back().push_back(index);
}

void ByteCodeVisitor::visit_UnaryOpASTNode(const UnaryOpASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  node.RHS->visit(*this);

  current_object.op_codes.push_back(UNARY_OP);
  switch (node.op.type)
  {
    case TOKEN_TYPE::PLUS:
    {
      current_object.op_codes.push_back(static_cast<OpCode>(POS));
      break;
    }
    case TOKEN_TYPE::MINUS:
    {
      current_object.op_codes.push_back(static_cast<OpCode>(NEG));
      break;
    }
    case TOKEN_TYPE::NOT:
    {
      current_object.op_codes.push_back(static_cast<OpCode>(NOT));
      break;
    }
    default: throw std::runtime_error("Unhandled token");
  }
}

void ByteCodeVisitor::visit_BinaryOpASTNode(const BinaryOpASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  node.LHS->visit(*this);
  node.RHS->visit(*this);

  current_object.op_codes.push_back(BINARY_OP);

  switch (node.op.type) {
    case TOKEN_TYPE::PLUS:
      current_object.op_codes.push_back(static_cast<OpCode>(ADD));
      break;

    case TOKEN_TYPE::TIMES:
      current_object.op_codes.push_back(static_cast<OpCode>(MUL));
      break;

    case TOKEN_TYPE::MINUS:
      current_object.op_codes.push_back(static_cast<OpCode>(SUB));
      break;

    case TOKEN_TYPE::SLASH:
      current_object.op_codes.push_back(static_cast<OpCode>(DIV));
      break;

    case TOKEN_TYPE::MOD:
      current_object.op_codes.push_back(static_cast<OpCode>(MOD));
      break;

    case TOKEN_TYPE::LT:
      current_object.op_codes.push_back(static_cast<OpCode>(LT));
      break;

    case TOKEN_TYPE::GT:
      current_object.op_codes.push_back(static_cast<OpCode>(GT));
      break;

    case TOKEN_TYPE::LQ:
      current_object.op_codes.push_back(static_cast<OpCode>(LQ));
      break;

    case TOKEN_TYPE::GQ:
      current_object.op_codes.push_back(static_cast<OpCode>(GQ));
      break;

    case TOKEN_TYPE::EQ:
      current_object.op_codes.push_back(static_cast<OpCode>(EQ));
      break;

    case TOKEN_TYPE::NEQ:
      current_object.op_codes.push_back(static_cast<OpCode>(NEQ));
      break;

    case TOKEN_TYPE::AND:
      current_object.op_codes.push_back(static_cast<OpCode>(AND));
      break;

    case TOKEN_TYPE::OR:
        current_object.op_codes.push_back(static_cast<OpCode>(OR));
        break;

    default: throw std::runtime_error("Unhandled token");
  }

}

void ByteCodeVisitor::visit_IdentifierASTNode(const IdentifierASTNode &node)
{
  // TODO: throw compilation error if variable doesnt exist

  // Variable is local if it's declared in current_object.locals,
  // otherwise it is global and should be put in current_object.names
  // However, we should TODO: handle access before declaration at compile-time

  auto& current_object = m_ObjectStack.back();

  std::string name = node.token.value;
  const auto var_name = std::string(node.token.value);

  // Firstly, check locals, if not found, check globals
  bool is_local = true;
  const auto locals_it = std::ranges::find(current_object.locals, var_name);
  std::size_t index = locals_it != current_object.locals.end() ? locals_it - current_object.locals.begin() : -1;

  if (index == -1)
  {
    const auto names_it = std::ranges::find(current_object.names, var_name);
    index = names_it != current_object.names.end() ? names_it - current_object.names.begin() : -1;
    is_local = false;
  }
  // it is not in locals nor names
  if (index == -1)
  {
    current_object.names.push_back(var_name);
    index = current_object.names.size() - 1;
  }

  if (next_identifier_as_store_name)
  {
    if (is_local) current_object.op_codes.push_back(STORE_LOCAL);
    else current_object.op_codes.push_back(STORE_NAME);

    current_object.op_codes.push_back(static_cast<OpCode>(index));
    next_identifier_as_store_name = false;
    return;
  }

  if (is_local) current_object.op_codes.push_back(LOAD_LOCAL);
  else current_object.op_codes.push_back(LOAD_NAME);

  current_object.op_codes.push_back(static_cast<OpCode>(index));
}

void ByteCodeVisitor::visit_IntegerASTNode(const IntegerASTNode &node)
{
  return emitConstantForNode<int, IntegerValue>(node.value);
}

void ByteCodeVisitor::visit_FloatASTNode(const FloatASTNode &node)
{
  emitConstantForNode<float, FloatValue>(node.value);
}

void ByteCodeVisitor::visit_BooleanASTNode(const BooleanASTNode &node)
{
  emitConstantForNode<bool, BooleanValue>(node.value);
}

void ByteCodeVisitor::visit_StringASTNode(const StringASTNode &node)
{
  emitConstantForNode<std::string, StringValue>(node.value);
}

void ByteCodeVisitor::visit_ArrayASTNode(const ArrayASTNode &node)
{
  for (const auto & value : std::ranges::reverse_view(node.values))
    value->visit(*this);

  auto& current_object = m_ObjectStack.back();
  current_object.op_codes.push_back(MAKE_ARR);
  current_object.op_codes.push_back(static_cast<OpCode>(node.values.size()));
}

void ByteCodeVisitor::visit_IfElseExpressionASTNode(const IfElseExpressionASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  node.condition->visit(*this);

  current_object.op_codes.push_back(JMP_IF_FALSE);
  current_object.op_codes.push_back(static_cast<OpCode>(0));
  const std::size_t jmp_if_false_operand_idx = current_object.op_codes.size() - 1;

  node.true_scope->visit(*this);

  if (node.false_scope)
  {
    current_object.op_codes.push_back(JMP);
    current_object.op_codes.push_back(static_cast<OpCode>(0));
    const std::size_t jmp_operand_idx = current_object.op_codes.size() - 1;

    const std::size_t else_start_ip = current_object.op_codes.size();
    current_object.op_codes[jmp_if_false_operand_idx] =
      static_cast<OpCode>(else_start_ip - (jmp_if_false_operand_idx + 1));

    node.false_scope->visit(*this);

    const std::size_t end_ip = current_object.op_codes.size();
    current_object.op_codes[jmp_operand_idx] =
      static_cast<OpCode>(end_ip - (jmp_operand_idx + 1));
  }
  else
  {
    const std::size_t end_ip = current_object.op_codes.size();
    current_object.op_codes[jmp_if_false_operand_idx] =
      static_cast<OpCode>(end_ip - (jmp_if_false_operand_idx + 1));
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
    current_object.op_codes.push_back(DEINIT_VAR);
    current_object.op_codes.push_back(static_cast<OpCode>(idx));
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
  const size_t cond_start = current_object.op_codes.size();

  // 3) condition
  if (node.condition) {
    node.condition->visit(*this);
  }
  current_object.op_codes.push_back(JMP_IF_FALSE);
  current_object.op_codes.push_back(static_cast<OpCode>(0)); // placeholder
  const size_t jmp_out_len_idx = current_object.op_codes.size() - 1; // index of placeholder

  // 4) body
  if (node.scope) {
    node.scope->visit(*this);
  }

  // 5) increment (runs after body each iteration)
  if (node.increment) {
    node.increment->visit(*this);
  }

  // 6) jump back to condition start (node_value)
  current_object.op_codes.push_back(JMP);
  current_object.op_codes.push_back(static_cast<OpCode>(0)); // placeholder
  const size_t jmp_back_len_idx = current_object.op_codes.size() - 1;

  // Patch backward jump:
  // immediate = target_index - (len_idx + 1)  ==> jump from after-immediate to cond_start
  {
    const int back = static_cast<int>(cond_start) - static_cast<int>(jmp_back_len_idx + 1);
    current_object.op_codes[jmp_back_len_idx] = static_cast<OpCode>(back);
  }

  // Patch "exit the loop" forward jump:
  // immediate = end_index - (len_idx + 1)  ==> jump from after-immediate to after-loop
  {
    const int out = static_cast<int>(current_object.op_codes.size()) - static_cast<int>(jmp_out_len_idx + 1);
    current_object.op_codes[jmp_out_len_idx] = static_cast<OpCode>(out);
  }
}

void ByteCodeVisitor::visit_WhileLoopASTNode(const WhileLoopASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  // 1) mark the start of the condition
  const size_t cond_start = current_object.op_codes.size();

  // 2) condition
  if (node.condition) {
    node.condition->visit(*this); // leaves truthy/falsy on stack
  }
  // If false -> jump to end (placeholder now, patch later)
  current_object.op_codes.push_back(JMP_IF_FALSE);
  current_object.op_codes.push_back(static_cast<OpCode>(0));
  const size_t jmp_out_len_idx = current_object.op_codes.size() - 1;

  // 3) body
  if (node.scope) {
    node.scope->visit(*this);
  }

  // 4) jump back to condition start
  current_object.op_codes.push_back(JMP);
  current_object.op_codes.push_back(static_cast<OpCode>(0)); // placeholder
  const size_t jmp_back_len_idx = current_object.op_codes.size() - 1;

  // --- patching ---

  // Backward jump: from after-immediate to cond_start
  {
    const int back = static_cast<int>(cond_start) - static_cast<int>(jmp_back_len_idx + 1);
    current_object.op_codes[jmp_back_len_idx] = static_cast<OpCode>(back);
  }

  // Exit jump: from after-immediate to after-loop (current end)
  {
    const int out = static_cast<int>(current_object.op_codes.size()) - static_cast<int>(jmp_out_len_idx + 1);
    current_object.op_codes[jmp_out_len_idx] = static_cast<OpCode>(out);
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

  const auto decl = static_cast<FunctionDeclASTNode*>(node.decl.get());
  const std::string name = decl->name.value;

  m_ObjectStack.push_back({ .name = name });

  decl->args->visit(*this);
  // FIXME: handle if no return -- return undefined
  node.body->visit(*this);

  auto f_code_object = std::move(m_ObjectStack.back());
  m_ObjectStack.pop_back();

  auto& current_object = m_ObjectStack.back();

  auto ptr = std::make_shared<CodeObject>(std::move(f_code_object));
  current_object.constants.push_back(std::make_shared<CodeObjectValue>(ptr));
  auto idx = current_object.constants.size() - 1;

  // TODO: handle name conflict
  current_object.names.push_back(name);
  std::size_t names_index = current_object.names.size() - 1;

  // 2. generate opcode to load the code object from constants and generate a function object
  current_object.op_codes.push_back(LOAD_CONST);
  current_object.op_codes.push_back(static_cast<OpCode>(idx));

  current_object.op_codes.push_back(MAKE_FUNC);
}

void ByteCodeVisitor::visit_ReturnStatementASTNode(const ReturnStatementASTNode &node)
{
  node.expr->visit(*this);

  auto& current_object = m_ObjectStack.back();
  current_object.op_codes.push_back(RETURN);
}

void ByteCodeVisitor::visit_FunctionCallASTNode(const FunctionCallASTNode &node)
{
  // Firstly, visit all regular params, then key-params
  std::size_t pos_args = 0, kw_args = 0;
  for (const auto& arg : node.args)
  {
    if (dynamic_cast<KeyParamExpressionASTNode*>(arg.get())) continue;
    arg->visit(*this);
    pos_args += 1;
  }
  for (const auto& arg : node.args)
  {
    if (!dynamic_cast<KeyParamExpressionASTNode*>(arg.get())) continue;
    arg->visit(*this);
    kw_args += 1;
  }

  node.base->visit(*this);

  auto& current_object = m_ObjectStack.back();

  if (is_kw_func)
  {
    is_kw_func = false;

    current_object.op_codes.push_back(KW_CALL);
    current_object.op_codes.push_back(static_cast<OpCode>(pos_args));
    current_object.op_codes.push_back(static_cast<OpCode>(kw_args));
  }
  else
  {
    current_object.op_codes.push_back(CALL);
    current_object.op_codes.push_back(static_cast<OpCode>(pos_args));
  }
}

void ByteCodeVisitor::visit_FunctionArgumentListASTNode(const FunctionArgumentListASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  for (const auto& arg : node.args)
  {
    const auto var_name = std::string(arg->name.value);
    const auto it = std::ranges::find(current_object.locals, var_name);

    std::size_t index = it != current_object.locals.end() ? it - current_object.locals.begin() : -1;
    if (index == -1)
    {
      current_object.locals.push_back(var_name);
      index = current_object.locals.size() - 1;
    }
    m_ScopeVars.back().push_back(index);

    const auto kind = arg->is_keyword
      ? ParamKind::KeywordOnly
      : ParamKind::PositionalOrKeyword;
    current_object.params.push_back(Parameter{
      .name        = var_name,
      .kind        = kind,
      .local_index = index
    });
  }
}

void ByteCodeVisitor::visit_MethodCallASTNode(const MethodCallASTNode &node)
{
  // Firstly, visit all regular params, then key-params
  std::size_t pos_args = 0, kw_args = 0;
  for (const auto& arg : node.args)
  {
    if (dynamic_cast<KeyParamExpressionASTNode*>(arg.get())) continue;
    arg->visit(*this);
    pos_args += 1;
  }
  for (const auto& arg : node.args)
  {
    if (!dynamic_cast<KeyParamExpressionASTNode*>(arg.get())) continue;
    arg->visit(*this);
    kw_args += 1;
  }

  node.base_expr->visit(*this);

  auto& current_object = m_ObjectStack.back();

  std::string method_name = node.name.value;
  const auto names_it = std::ranges::find(current_object.names, method_name);
  std::size_t index = names_it != current_object.names.end() ? names_it - current_object.names.begin() : -1;
  if (index == -1)
  {
    current_object.names.push_back(method_name);
    index = current_object.names.size() - 1;
  }

  if (is_kw_func)
  {
    is_kw_func = false;

    current_object.op_codes.push_back(KW_CALL_METHOD);
    current_object.op_codes.push_back(static_cast<OpCode>(index));
    current_object.op_codes.push_back(static_cast<OpCode>(pos_args));
    current_object.op_codes.push_back(static_cast<OpCode>(kw_args));
  }
  else
  {
    current_object.op_codes.push_back(CALL_METHOD);
    current_object.op_codes.push_back(static_cast<OpCode>(index));
    current_object.op_codes.push_back(static_cast<OpCode>(pos_args));
  }
}

// LOAD_CONST 0 (func)
// LOAD_CONST 1 (1)
// LOAD_CONST 2 (2)
// LOAD_CONST 3 ("+")
// LOAD_CONST 4 ("op")
// CALL_KW 2 1 (pos_args, kw_args)

void ByteCodeVisitor::visit_KeyParamExpressionASTNode(const KeyParamExpressionASTNode &node)
{
  is_kw_func = true;

  node.expression->visit(*this);
  emitConstantForNode<std::string, StringValue>(node.identifier.value);
}

}
