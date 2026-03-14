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

std::shared_ptr<CodeObject> ByteCodeVisitor::visit_RootASTNode(const RootASTNode &node)
{
  m_ObjectStack.push_back(std::make_shared<CodeObject>(CodeObject{ .name = "__main__" }));
  m_CompileContextStack.push_back({
    .resolution_result = m_ResolutionResult,
    .co = m_ObjectStack.back(),
  });

  for (const auto& child_node : node.nodes)
  {
    child_node->visit(*this);
  }
  m_CompileContextStack.pop_back();
  m_ObjectStack.back()->op_codes.push_back(HALT);

  return m_ObjectStack.back();
}

void ByteCodeVisitor::visit(const VariableASTNode &node)
{
  const auto current_object = m_ObjectStack.back();

  const auto& symbol_id = m_ResolutionResult.node_to_symbol.at(node.id);
  const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);
  const std::size_t locals_idx = m_CompileContextStack.back().AllocateLocal(symbol_id, symbol.name);

  if (node.value) { node.value->visit(*this); }
  else { current_object->op_codes.push_back(LOAD_UNDEF); }

  current_object->op_codes.push_back(INIT_VAR);
  current_object->op_codes.push_back(static_cast<OpCode>(locals_idx));
}

void ByteCodeVisitor::visit(const UnaryOpASTNode &node)
{
  const auto current_object = m_ObjectStack.back();

  node.RHS->visit(*this);

  current_object->op_codes.push_back(UNARY_OP);
  switch (node.op.type)
  {
    case TOKEN_TYPE::PLUS:
    {
      current_object->op_codes.push_back(static_cast<OpCode>(POS));
      break;
    }
    case TOKEN_TYPE::MINUS:
    {
      current_object->op_codes.push_back(static_cast<OpCode>(NEG));
      break;
    }
    case TOKEN_TYPE::NOT:
    {
      current_object->op_codes.push_back(static_cast<OpCode>(NOT));
      break;
    }
    default: throw std::runtime_error("Unhandled token");
  }
}

void ByteCodeVisitor::visit(const BinaryOpASTNode &node)
{
  const auto current_object = m_ObjectStack.back();

  node.LHS->visit(*this);
  node.RHS->visit(*this);

  current_object->op_codes.push_back(BINARY_OP);

  switch (node.op.type) {
    case TOKEN_TYPE::PLUS:
      current_object->op_codes.push_back(static_cast<OpCode>(ADD));
      break;

    case TOKEN_TYPE::TIMES:
      current_object->op_codes.push_back(static_cast<OpCode>(MUL));
      break;

    case TOKEN_TYPE::MINUS:
      current_object->op_codes.push_back(static_cast<OpCode>(SUB));
      break;

    case TOKEN_TYPE::SLASH:
      current_object->op_codes.push_back(static_cast<OpCode>(DIV));
      break;

    case TOKEN_TYPE::MOD:
      current_object->op_codes.push_back(static_cast<OpCode>(MOD));
      break;

    case TOKEN_TYPE::LT:
      current_object->op_codes.push_back(static_cast<OpCode>(LT));
      break;

    case TOKEN_TYPE::GT:
      current_object->op_codes.push_back(static_cast<OpCode>(GT));
      break;

    case TOKEN_TYPE::LQ:
      current_object->op_codes.push_back(static_cast<OpCode>(LQ));
      break;

    case TOKEN_TYPE::GQ:
      current_object->op_codes.push_back(static_cast<OpCode>(GQ));
      break;

    case TOKEN_TYPE::EQ:
      current_object->op_codes.push_back(static_cast<OpCode>(EQ));
      break;

    case TOKEN_TYPE::NEQ:
      current_object->op_codes.push_back(static_cast<OpCode>(NEQ));
      break;

    case TOKEN_TYPE::AND:
      current_object->op_codes.push_back(static_cast<OpCode>(AND));
      break;

    case TOKEN_TYPE::OR:
      current_object->op_codes.push_back(static_cast<OpCode>(OR));
      break;

    default: throw std::runtime_error("Unhandled token");
  }

}

void ByteCodeVisitor::visit(const IdentifierASTNode &node)
{
	// FIXME: Resolver is wrong here, since for example 
	// const SEPARATOR = " ";
	// fn get_separator() { return SEPARATOR }
	// results in SEPARATOR being local to this function!
	
  const auto current_object = m_ObjectStack.back();
  const std::size_t symbol_id = m_ResolutionResult.node_to_symbol.at(node.id);
  const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);

  if (symbol.IsLocal())
  {
    std::size_t index = m_CompileContextStack.back().GetLocalIndex(symbol_id);

    current_object->op_codes.push_back( next_identifier_as_store_name ? STORE_LOCAL : LOAD_LOCAL);
    current_object->op_codes.push_back(static_cast<OpCode>(index));
    next_identifier_as_store_name = false;
    return;
  }
  std::size_t index = m_CompileContextStack.back().GetOrCreateNameIndex(symbol_id, symbol.name);

  current_object->op_codes.push_back( next_identifier_as_store_name ? STORE_NAME : LOAD_NAME);
  current_object->op_codes.push_back(static_cast<OpCode>(index));
  next_identifier_as_store_name = false;
}

void ByteCodeVisitor::visit(const IntegerASTNode &node)
{
  return emitConstantForNode<int, IntegerValue>(node.value);
}

void ByteCodeVisitor::visit(const FloatASTNode &node)
{
  emitConstantForNode<float, FloatValue>(node.value);
}

void ByteCodeVisitor::visit(const BooleanASTNode &node)
{
  emitConstantForNode<bool, BooleanValue>(node.value);
}

void ByteCodeVisitor::visit(const StringASTNode &node)
{
  emitConstantForNode<std::string, StringValue>(node.value);
}

void ByteCodeVisitor::visit(const ArrayASTNode &node)
{
  for (const auto& value : std::ranges::reverse_view(node.values))
    value->visit(*this);

  const auto current_object = m_ObjectStack.back();
  current_object->op_codes.push_back(MAKE_ARR);
  current_object->op_codes.push_back(static_cast<OpCode>(node.values.size()));
}

void ByteCodeVisitor::visit(const DictASTNode &node)
{
  std::size_t len = node.keys.size();

  for (std::size_t i = 0; i < len; i++)
  {
    node.keys[i]->visit(*this);
    node.values[i]->visit(*this);
  }

  const auto current_object = m_ObjectStack.back();
  current_object->op_codes.push_back(MAKE_DICT);
  current_object->op_codes.push_back(static_cast<OpCode>(len));
}

void ByteCodeVisitor::visit(const IfElseExpressionASTNode &node)
{
  auto current_object = m_ObjectStack.back();

  node.condition->visit(*this);

  current_object->op_codes.push_back(JMP_IF_FALSE);
  current_object->op_codes.push_back(static_cast<OpCode>(0));
  const std::size_t jmp_if_false_operand_idx = current_object->op_codes.size() - 1;

  node.true_scope->visit(*this);

  if (node.false_scope)
  {
    current_object->op_codes.push_back(JMP);
    current_object->op_codes.push_back(static_cast<OpCode>(0));
    const std::size_t jmp_operand_idx = current_object->op_codes.size() - 1;

    const std::size_t else_start_ip = current_object->op_codes.size();
    current_object->op_codes[jmp_if_false_operand_idx] =
      static_cast<OpCode>(else_start_ip - (jmp_if_false_operand_idx + 1));

    node.false_scope->visit(*this);

    const std::size_t end_ip = current_object->op_codes.size();
    current_object->op_codes[jmp_operand_idx] =
      static_cast<OpCode>(end_ip - (jmp_operand_idx + 1));
  }
  else
  {
    const std::size_t end_ip = current_object->op_codes.size();
    current_object->op_codes[jmp_if_false_operand_idx] =
      static_cast<OpCode>(end_ip - (jmp_if_false_operand_idx + 1));
  }
}

void ByteCodeVisitor::visit(const ScopeASTNode &node)
{
  const auto current_object = m_ObjectStack.back();

  for (const auto& child_node : node.nodes)
  {
    child_node->visit(*this);
  }
  for (std::size_t idx = 0; idx < current_object->locals.size(); idx++)
  {
    current_object->op_codes.push_back(DEINIT_VAR);
    current_object->op_codes.push_back(static_cast<OpCode>(idx));
  }
}

void ByteCodeVisitor::visit(const ForLoopASTNode &node)
{
  const auto current_object = m_ObjectStack.back();
  m_CompileContextStack.push_back({
    .resolution_result = m_ResolutionResult,
    .co = m_ObjectStack.back(),
  });

  node.declaration->visit(*this);

  const std::size_t cond_start = current_object->op_codes.size();
  node.condition->visit(*this);

  current_object->op_codes.push_back(JMP_IF_FALSE);
  current_object->op_codes.push_back(static_cast<OpCode>(0)); // placeholder
  const size_t jmp_out_len_idx = current_object->op_codes.size() - 1; // index of placeholder

  node.scope->visit(*this);
  node.increment->visit(*this);

  // 6) jump back to condition start (node_value)
  current_object->op_codes.push_back(JMP);
  current_object->op_codes.push_back(static_cast<OpCode>(0)); // placeholder
  const std::size_t jmp_back_len_idx = current_object->op_codes.size() - 1;

  // Patch backward jump:
  // immediate = target_index - (len_idx + 1)  ==> jump from after-immediate to cond_start
  const int back = static_cast<int>(cond_start) - static_cast<int>(jmp_back_len_idx + 1);
  current_object->op_codes[jmp_back_len_idx] = static_cast<OpCode>(back);

  // Patch "exit the loop" forward jump:
  // immediate = end_index - (len_idx + 1)  ==> jump from after-immediate to after-loop
  const int out = static_cast<int>(current_object->op_codes.size()) - static_cast<int>(jmp_out_len_idx + 1);
  current_object->op_codes[jmp_out_len_idx] = static_cast<OpCode>(out);

  m_CompileContextStack.pop_back();
}

void ByteCodeVisitor::visit(const ForEachLoopASTNode &node)
{
  const auto current_object = m_ObjectStack.back();
  const std::size_t symbol_id = m_ResolutionResult.node_to_symbol.at(node.id);
  const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);

  m_CompileContextStack.push_back({
    .resolution_result = m_ResolutionResult,
    .co = m_ObjectStack.back(),
  });

  std::size_t index = m_CompileContextStack.back().AllocateLocal(symbol_id, symbol.name);

  current_object->op_codes.push_back(LOAD_UNDEF);

  current_object->op_codes.push_back(INIT_VAR);
  current_object->op_codes.push_back(static_cast<OpCode>(index));

  node.iterable_expr->visit(*this);
  current_object->op_codes.push_back(GET_ITER);

  current_object->op_codes.push_back(FOR_ITER);
  current_object->op_codes.push_back(static_cast<OpCode>(0));
  auto idx = current_object->op_codes.size() - 1;

  current_object->op_codes.push_back(STORE_LOCAL);
  current_object->op_codes.push_back(static_cast<OpCode>(index));

  node.scope->visit(*this);

  current_object->op_codes.at(idx) = static_cast<OpCode>(current_object->op_codes.size() - idx + 1);

  current_object->op_codes.push_back(JMP);
  current_object->op_codes.push_back(static_cast<OpCode>(idx - current_object->op_codes.size() - 2));

  m_CompileContextStack.pop_back();
}

void ByteCodeVisitor::visit(const WhileLoopASTNode &node)
{
  const auto current_object = m_ObjectStack.back();

  const std::size_t cond_start = current_object->op_codes.size();

  node.condition->visit(*this);

  current_object->op_codes.push_back(JMP_IF_FALSE);
  current_object->op_codes.push_back(static_cast<OpCode>(0));
  const std::size_t jmp_out_len_idx = current_object->op_codes.size() - 1;

  node.scope->visit(*this);

  current_object->op_codes.push_back(JMP);
  current_object->op_codes.push_back(static_cast<OpCode>(0)); // placeholder
  const std::size_t jmp_back_len_idx = current_object->op_codes.size() - 1;

  const std::size_t back = cond_start - jmp_back_len_idx + 1;
  current_object->op_codes[jmp_back_len_idx] = static_cast<OpCode>(back);

  const std::size_t out = current_object->op_codes.size() - jmp_out_len_idx + 1;
  current_object->op_codes[jmp_out_len_idx] = static_cast<OpCode>(out);
}

void ByteCodeVisitor::visit(const StatementASTNode &node)
{
  // for now just handle assignment to variable statement
  // TODO: handle method and property assignment

  // push rhs to stack
  node.RHS->visit(*this);

  next_identifier_as_store_name = true;

  node.base->visit(*this);
}

void ByteCodeVisitor::visit(const FunctionASTNode &node)
{
  const auto current_object = m_ObjectStack.back();
  const std::size_t symbol_id = m_ResolutionResult.node_to_symbol.at(node.id);
  const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);

  m_ObjectStack.push_back(std::make_shared<CodeObject>(CodeObject{ .name = symbol.name }));
  m_CompileContextStack.push_back({
    .resolution_result = m_ResolutionResult,
    .co = m_ObjectStack.back(),
  });

  node.decl->visit(*this);
  node.body->visit(*this);

  auto f_code_object = m_ObjectStack.back();
  m_ObjectStack.pop_back();
	m_CompileContextStack.pop_back();

  current_object->constants.push_back(std::make_shared<CodeObjectValue>(f_code_object));
  const std::size_t idx = current_object->constants.size() - 1;

  m_CompileContextStack.back().GetOrCreateNameIndex(symbol_id, symbol.name);

  current_object->op_codes.push_back(LOAD_CONST);
  current_object->op_codes.push_back(static_cast<OpCode>(idx));

  current_object->op_codes.push_back(MAKE_FUNC);

}

void ByteCodeVisitor::visit(const FunctionDeclASTNode &node)
{
  node.args->visit(*this);
}

void ByteCodeVisitor::visit(const ReturnStatementASTNode &node)
{
  node.expr->visit(*this);

  auto& current_object = m_ObjectStack.back();
  current_object->op_codes.push_back(RETURN);
}

void ByteCodeVisitor::visit(const FunctionCallASTNode &node)
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

  const auto current_object = m_ObjectStack.back();

  if (is_kw_func)
  {
    is_kw_func = false;

    current_object->op_codes.push_back(KW_CALL);
    current_object->op_codes.push_back(static_cast<OpCode>(pos_args));
    current_object->op_codes.push_back(static_cast<OpCode>(kw_args));
  }
  else
  {
    current_object->op_codes.push_back(CALL);
    current_object->op_codes.push_back(static_cast<OpCode>(pos_args));
  }
}

void ByteCodeVisitor::visit(const FunctionArgumentListASTNode &node)
{
  const auto current_object = m_ObjectStack.back();

  for (const auto& arg : node.args)
  {
    const auto& symbol_id = m_ResolutionResult.node_to_symbol.at(arg->id);
    const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);
    const std::size_t idx = m_CompileContextStack.back().AllocateLocal(symbol_id, symbol.name);

    const auto kind = arg->is_keyword
      ? ParamKind::KeywordOnly
      : ParamKind::PositionalOrKeyword;
    m_CompileContextStack.back().CreateParameter(Parameter{
      .name        = symbol.name,
      .kind        = kind,
      .local_index = idx
    });
  }
}

void ByteCodeVisitor::visit(const MethodCallASTNode &node)
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

  const auto current_object = m_ObjectStack.back();
  const auto& symbol_id = m_ResolutionResult.node_to_symbol.at(node.id);
  const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);

  const std::size_t index = m_CompileContextStack.back().GetOrCreateNameIndex(symbol_id, symbol.name);

  if (is_kw_func)
  {
    is_kw_func = false;

    current_object->op_codes.push_back(KW_CALL_METHOD);
    current_object->op_codes.push_back(static_cast<OpCode>(index));
    current_object->op_codes.push_back(static_cast<OpCode>(pos_args));
    current_object->op_codes.push_back(static_cast<OpCode>(kw_args));
  }
  else
  {
    current_object->op_codes.push_back(CALL_METHOD);
    current_object->op_codes.push_back(static_cast<OpCode>(index));
    current_object->op_codes.push_back(static_cast<OpCode>(pos_args));
  }
}

void ByteCodeVisitor::visit(const GetPropertyASTNode &node)
{
  node.base_expr->visit(*this);

  const auto current_object = m_ObjectStack.back();
  const auto& symbol_id = m_ResolutionResult.node_to_symbol.at(node.id);
  const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);

  const std::size_t index = m_CompileContextStack.back().GetOrCreateNameIndex(symbol_id, symbol.name);

  current_object->op_codes.push_back(GET_PROPERTY);
  current_object->op_codes.push_back(static_cast<OpCode>(index));
}

void ByteCodeVisitor::visit(const SetPropertyASTNode &node)
{
  node.RHS->visit(*this);
  node.base_expr->visit(*this);

  const auto current_object = m_ObjectStack.back();
  const auto& symbol_id = m_ResolutionResult.node_to_symbol.at(node.id);
  const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);

  const std::size_t index = m_CompileContextStack.back().GetOrCreateNameIndex(symbol_id, symbol.name);

  current_object->op_codes.push_back(SET_PROPERTY);
  current_object->op_codes.push_back(static_cast<OpCode>(index));
}

void ByteCodeVisitor::visit(const ClassASTNode &node)
{
  const auto current_object = m_ObjectStack.back();

  for (const auto& _method_node : node.member_functions)
  {
    const auto& symbol_id = m_ResolutionResult.node_to_symbol.at(_method_node->id);
    const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);

    const auto method_node = static_cast<FunctionASTNode*>(_method_node.get());

    m_ObjectStack.push_back(std::make_shared<CodeObject>(CodeObject{ .name = symbol.name }));
    m_CompileContextStack.push_back({
      .resolution_result = m_ResolutionResult,
      .co = m_ObjectStack.back(),
    });

    method_node->decl->visit(*this);
    method_node->body->visit(*this);

    m_CompileContextStack.pop_back();
    const auto method_code_object = m_ObjectStack.back();
    m_ObjectStack.pop_back();

    current_object->constants.push_back(mk_code_obj(method_code_object));
    const std::size_t code_idx = current_object->constants.size() - 1;

    current_object->op_codes.push_back(LOAD_CONST);
    current_object->op_codes.push_back(static_cast<OpCode>(code_idx));

    emitConstantForNode<std::string, StringValue>(symbol.name);
  }

  const auto& symbol_id = m_ResolutionResult.node_to_symbol.at(node.id);
  const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);

  const std::size_t name_index = m_CompileContextStack.back().GetOrCreateNameIndex(symbol_id, symbol.name);

  current_object->op_codes.push_back(MAKE_TYPE);
  current_object->op_codes.push_back(static_cast<OpCode>(name_index));
  current_object->op_codes.push_back(static_cast<OpCode>(node.member_functions.size()));
}

void ByteCodeVisitor::visit(const ExportASTNode &node)
{
  const auto current_object = m_ObjectStack.back();

  for (const auto& var : node.variables)
  {
    const auto& symbol_id = m_ResolutionResult.node_to_symbol.at(var->id);
    const auto& symbol = m_ResolutionResult.symbols.at(symbol_id);

    const std::size_t index = symbol.IsLocal() ?
			 m_CompileContextStack.back().GetLocalIndex(symbol_id) :
			 m_CompileContextStack.back().GetOrCreateNameIndex(symbol_id, symbol.name);

    current_object->exports.push_back((Export{
      .kind = symbol.IsLocal() ? ExportKind::LOCAL : ExportKind::NAME,
      .index = index,
      .name = symbol.name,
    }));
  }
}

void ByteCodeVisitor::visit(const ImportASTNode &node)
{
  const auto current_object = m_ObjectStack.back();
  const std::size_t scope_id = m_ResolutionResult.node_to_scope.at(node.id);
  const auto& scope = m_ResolutionResult.scopes.at(scope_id);

  node.module->visit(*this);

  current_object->op_codes.push_back(LOAD_MODULE);

  for (const auto& identifier : node.identifiers)
  {
    const std::size_t symbol_id = scope.name_to_symbol.at(identifier.value);
    const std::size_t index = m_CompileContextStack.back().GetOrCreateNameIndex(symbol_id, identifier.value);

    current_object->op_codes.push_back(IMPORT_NAME);
    current_object->op_codes.push_back(static_cast<OpCode>(index));
  }

  current_object->op_codes.push_back(POP);
}

void ByteCodeVisitor::visit(const IndexASTNode &node)
{
  node.base_expr->visit(*this);
  node.index_expr->visit(*this);

  const auto current_object = m_ObjectStack.back();

  if (m_IsSetIndex)
  {
    m_IsSetIndex = false;
    return;
  }

  current_object->op_codes.push_back(GET_INDEX);
}

void ByteCodeVisitor::visit(const StatementIndexASTNode &node)
{
  m_IsSetIndex = true;
  node.identifier->visit(*this);
  node.RHS->visit(*this);

  const auto current_object = m_ObjectStack.back();
  current_object->op_codes.push_back(SET_INDEX);
}

void ByteCodeVisitor::visit(const KeyParamExpressionASTNode &node)
{
  is_kw_func = true;

  node.expression->visit(*this);
  emitConstantForNode<std::string, StringValue>(node.identifier.value);
}

}
