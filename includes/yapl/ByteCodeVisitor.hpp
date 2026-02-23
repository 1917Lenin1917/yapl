//
// Created by Максим Литвиненко on 27.08.2025.
//

#pragma once
#include "CodeObject.hpp"

namespace yapl {
class FunctionArgumentListASTNode;
class FunctionCallASTNode;
class ReturnStatementASTNode;
class FunctionASTNode;
class FunctionDeclASTNode;
class WhileLoopASTNode;
class StatementASTNode;
class ForLoopASTNode;
class ScopeASTNode;
class IfElseExpressionASTNode;
class IdentifierASTNode;
class StringASTNode;
class BooleanASTNode;
class IntegerASTNode;
class FloatASTNode;
class UnaryOpASTNode;
class BinaryOpASTNode;
class VariableASTNode;
class RootASTNode;
class KeyParamExpressionASTNode;
class MethodCallASTNode;
class ArrayASTNode;
class GetPropertyASTNode;
class SetPropertyASTNode;

class ByteCodeVisitor final
{
public:
  explicit ByteCodeVisitor() = default;

  CodeObject visit_RootASTNode(const RootASTNode &node);
  void visit_VariableASTNode(const VariableASTNode &node);
  void visit_UnaryOpASTNode(const UnaryOpASTNode &node);
  void visit_BinaryOpASTNode(const BinaryOpASTNode &node);
  void visit_IdentifierASTNode(const IdentifierASTNode &node);


  void visit_IntegerASTNode(const IntegerASTNode &node);
  void visit_FloatASTNode(const FloatASTNode &node);
  void visit_BooleanASTNode(const BooleanASTNode &node);
  void visit_StringASTNode(const StringASTNode &node);
  void visit_ArrayASTNode(const ArrayASTNode &node);

  void visit_IfElseExpressionASTNode(const IfElseExpressionASTNode &node);
  void visit_ScopeASTNode(const ScopeASTNode &node);
  void visit_ForLoopASTNode(const ForLoopASTNode &node);
  void visit_WhileLoopASTNode(const WhileLoopASTNode &node);
  void visit_StatementASTNode(const StatementASTNode &node);
  void visit_FunctionASTNode(const FunctionASTNode &node);
  void visit_ReturnStatementASTNode(const ReturnStatementASTNode &node);
  void visit_FunctionCallASTNode(const FunctionCallASTNode& node);
  void visit_FunctionArgumentListASTNode(const FunctionArgumentListASTNode &node);
  void visit_MethodCallASTNode(const MethodCallASTNode &node);
  void visit_GetPropertyASTNode(const GetPropertyASTNode &node);
  void visit_SetPropertyASTNode(const SetPropertyASTNode &node);

  void visit_KeyParamExpressionASTNode(const KeyParamExpressionASTNode &node);


  template <typename NodeValue, typename ValueType>
  void emitConstantForNode(const NodeValue &node_value)
  {
    auto &current_object = m_ObjectStack.back();
    auto nodeValue = node_value;

    auto it = std::ranges::find_if(
        current_object.constants,
        [nodeValue](const std::shared_ptr<Value> &value)
        {
          auto typedValue = dynamic_cast<ValueType *>(value.get());
          return typedValue && typedValue->value == nodeValue;
        });

    std::size_t index =
        it != current_object.constants.end()
            ? static_cast<std::size_t>(it - current_object.constants.begin())
            : static_cast<std::size_t>(-1);

    if (index == static_cast<std::size_t>(-1))
    {
      current_object.constants.push_back(
          std::make_shared<ValueType>(node_value));
      index = current_object.constants.size() - 1;
    }

    current_object.op_codes.push_back(LOAD_CONST);
    current_object.op_codes.push_back(static_cast<OpCode>(index));

  }
  /*
  Objec visit_ArrayASTNode(const ArrayASTNode &node) override;
  Objec visit_DictASTNode(const DictASTNode &node) override;
  Objec visit_IndexASTNode(const IndexASTNode &node) override;
  Objec visit_UnaryOpASTNode(const UnaryOpASTNode &node) override;
  Objec visit_ImportASTNode(const ImportASTNode &node) override;
  Objec visit_ExportASTNode(const ExportASTNode &node) override;
  Objec visit_StatementIndexASTNode(const StatementIndexASTNode &node) override;
  Objec visit_ForEachLoopASTNode(const ForEachLoopASTNode &node) override;
  Objec visit_StarredExpressionASTNode(const StarredExpressionASTNode &node) override;
  Objec visit_ClassASTNode(const ClassASTNode &node) override;
  */

private:
  std::vector<CodeObject> m_ObjectStack;

  bool next_identifier_as_store_name = false;
  bool is_kw_func = false;
  // TODO: change?
  std::vector<std::vector<std::size_t>> m_ScopeVars;
};


}
