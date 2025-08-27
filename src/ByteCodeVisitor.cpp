//
// Created by Максим Литвиненко on 27.08.2025.
//

#include "yapl/ByteCodeVisitor.hpp"
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

  for (const auto& child_node : node.nodes)
  {
    child_node->visit(*this);
  }
  m_ObjectStack.back().OpCodes.push_back(HALT);

  return m_ObjectStack.back();

}

void ByteCodeVisitor::visit_VariableASTNode(const VariableASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

}

void ByteCodeVisitor::visit_BinaryOpASTNode(const BinaryOpASTNode &node)
{
  auto& current_object = m_ObjectStack.back();

  // why do i return the index???
  const auto idx1 = node.LHS->visit(*this);
  const auto idx2 = node.RHS->visit(*this);

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

}
