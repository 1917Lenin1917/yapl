//
// Created by Максим Литвиненко on 27.08.2025.
//

#pragma once
#include "BaseVisitor.hpp"
#include "CodeObject.hpp"

namespace yapl {

class ByteCodeVisitor final : public Visitor
{
public:
  explicit ByteCodeVisitor() = default;

  CodeObject visit_RootASTNode(const RootASTNode &node);
  void visit(const VariableASTNode &node) override;
  void visit(const UnaryOpASTNode &node) override;
  void visit(const BinaryOpASTNode &node) override;
  void visit(const IdentifierASTNode &node) override;


  void visit(const IntegerASTNode &node) override;
  void visit(const FloatASTNode &node) override;
  void visit(const BooleanASTNode &node) override;
  void visit(const StringASTNode &node) override;
  void visit(const ArrayASTNode &node) override;
  void visit(const DictASTNode &node) override;

  void visit(const IfElseExpressionASTNode &node) override;
  void visit(const ScopeASTNode &node) override;
  void visit(const ForLoopASTNode &node) override;
  void visit(const ForEachLoopASTNode &node) override;
  void visit(const WhileLoopASTNode &node) override;
  void visit(const StatementASTNode &node) override;
  void visit(const FunctionASTNode &node) override;
  void visit(const ReturnStatementASTNode &node) override;
  void visit(const FunctionCallASTNode& node) override;
  void visit(const FunctionArgumentListASTNode &node) override;
  void visit(const MethodCallASTNode &node) override;
  void visit(const GetPropertyASTNode &node) override;
  void visit(const SetPropertyASTNode &node) override;
  void visit(const ClassASTNode &node) override;
  void visit(const ExportASTNode &node) override;
  void visit(const ImportASTNode &node) override;
  void visit(const IndexASTNode &node) override;
  void visit(const StatementIndexASTNode &node) override;

  void visit(const KeyParamExpressionASTNode &node) override;


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
  Objec visit_ForEachLoopASTNode(const ForEachLoopASTNode &node) override;
  Objec visit_StarredExpressionASTNode(const StarredExpressionASTNode &node) override;
  */

private:
  std::vector<CodeObject> m_ObjectStack;

  bool next_identifier_as_store_name = false;
  bool is_kw_func = false;
  // TODO: change?
  std::vector<std::vector<std::size_t>> m_ScopeVars;
  bool m_IsSetIndex = false;
};


}
