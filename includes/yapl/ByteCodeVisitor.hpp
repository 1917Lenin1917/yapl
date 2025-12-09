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
class LiteralASTNode;
class BinaryOpASTNode;
class VariableASTNode;
class RootASTNode;


class ByteCodeVisitor final
{
public:
  explicit ByteCodeVisitor() = default;

  CodeObject visit_RootASTNode(const RootASTNode &node);
  void visit_VariableASTNode(const VariableASTNode &node);
  void visit_BinaryOpASTNode(const BinaryOpASTNode &node);
  void visit_IdentifierASTNode(const IdentifierASTNode &node);
  std::size_t visit_IntegerASTNode(const IntegerASTNode &node);
  void visit_FloatASTNode(const FloatASTNode &node);
  void visit_BooleanASTNode(const BooleanASTNode &node);
  void visit_StringASTNode(const StringASTNode &node);
  void visit_IfElseExpressionASTNode(const IfElseExpressionASTNode &node);
  void visit_ScopeASTNode(const ScopeASTNode &node);
  void visit_ForLoopASTNode(const ForLoopASTNode &node);
  void visit_WhileLoopASTNode(const WhileLoopASTNode &node);
  void visit_StatementASTNode(const StatementASTNode &node);
  void visit_FunctionASTNode(const FunctionASTNode &node);
  void visit_ReturnStatementASTNode(const ReturnStatementASTNode &node);
  void visit_FunctionCallASTNode(const FunctionCallASTNode& node);
  void visit_FunctionArgumentListASTNode(const FunctionArgumentListASTNode &node);

  /*
  Objec visit_LiteralASTNode(const LiteralASTNode &node) override;
  Objec visit_IntegerASTNode(const IntegerASTNode &node) override;
  Objec visit_FloatASTNode(const FloatASTNode &node) override;
  Objec visit_BooleanASTNode(const BooleanASTNode &node) override;
  Objec visit_StringASTNode(const StringASTNode &node) override;
  Objec visit_IdentifierASTNode(const IdentifierASTNode &node) override;
  Objec visit_ArrayASTNode(const ArrayASTNode &node) override;
  Objec visit_DictASTNode(const DictASTNode &node) override;
  Objec visit_IndexASTNode(const IndexASTNode &node) override;
  Objec visit_UnaryOpASTNode(const UnaryOpASTNode &node) override;
  Objec visit_BinaryOpASTNode(const BinaryOpASTNode &node) override;
  Objec visit_ImportASTNode(const ImportASTNode &node) override;
  Objec visit_ExportASTNode(const ExportASTNode &node) override;
  Objec visit_StatementIndexASTNode(const StatementIndexASTNode &node) override;
  Objec visit_IfElseExpressionASTNode(const IfElseExpressionASTNode &node) override;
  Objec visit_ForEachLoopASTNode(const ForEachLoopASTNode &node) override;
  Objec visit_ReturnStatementASTNode(const ReturnStatementASTNode &node) override;
  Objec visit_ScopeASTNode(const ScopeASTNode &node) override;
  Objec visit_FunctionArgumentASTNode(const FunctionArgumentASTNode &node) override;
  Objec visit_FunctionArgumentListASTNode(const FunctionArgumentListASTNode &node) override;
  Objec visit_FunctionDeclASTNode(const FunctionDeclASTNode &node) override;
  Objec visit_FunctionCallASTNode(const FunctionCallASTNode& node) override;
  Objec visit_MethodCallASTNode(const MethodCallASTNode &node) override;
  Objec visit_GetPropertyASTNode(const GetPropertyASTNode &node) override;
  Objec visit_SetPropertyASTNode(const SetPropertyASTNode &node) override;
  Objec visit_StarredExpressionASTNode(const StarredExpressionASTNode &node) override;
  Objec visit_FunctionASTNode(const FunctionASTNode &node) override;
  Objec visit_ClassASTNode(const ClassASTNode &node) override;
  Objec visit_BuiltinCustomVisitFunctionASTNode(const BuiltinCustomVisitFunctionASTNode &node) override;
  */

private:
  std::vector<CodeObject> m_ObjectStack;

  bool next_identifier_as_store_name = false;
  // TODO: change?
  std::vector<std::vector<std::size_t>> m_ScopeVars;
};

}
