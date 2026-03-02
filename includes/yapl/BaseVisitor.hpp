//
// Created by lenin on 01.03.2026.
//

#pragma once

namespace yapl {
class ForEachLoopASTNode;
class DictASTNode;

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
class ClassASTNode;
class ExportASTNode;
class ImportASTNode;
class IndexASTNode;
class StatementIndexASTNode;

class Visitor
{
public:
  virtual ~Visitor() = default;

  virtual void visit(const VariableASTNode &node) = 0;
  virtual void visit(const UnaryOpASTNode &node) = 0;
  virtual void visit(const BinaryOpASTNode &node) = 0;
  virtual void visit(const IdentifierASTNode &node) = 0;


  virtual void visit(const IntegerASTNode &node) = 0;
  virtual void visit(const FloatASTNode &node) = 0;
  virtual void visit(const BooleanASTNode &node) = 0;
  virtual void visit(const StringASTNode &node) = 0;
  virtual void visit(const ArrayASTNode &node) = 0;
  virtual void visit(const DictASTNode &node) = 0;

  virtual void visit(const IfElseExpressionASTNode &node) = 0;
  virtual void visit(const ScopeASTNode &node) = 0;
  virtual void visit(const ForLoopASTNode &node) = 0;
  virtual void visit(const ForEachLoopASTNode &node) = 0;
  virtual void visit(const WhileLoopASTNode &node) = 0;
  virtual void visit(const StatementASTNode &node) = 0;
  virtual void visit(const FunctionASTNode &node) = 0;
  virtual void visit(const ReturnStatementASTNode &node) = 0;
  virtual void visit(const FunctionCallASTNode& node) = 0;
  virtual void visit(const FunctionArgumentListASTNode &node) = 0;
  virtual void visit(const MethodCallASTNode &node) = 0;
  virtual void visit(const GetPropertyASTNode &node) = 0;
  virtual void visit(const SetPropertyASTNode &node) = 0;
  virtual void visit(const ClassASTNode &node) = 0;
  virtual void visit(const ExportASTNode &node) = 0;
  virtual void visit(const ImportASTNode &node) = 0;
  virtual void visit(const IndexASTNode &node) = 0;
  virtual void visit(const StatementIndexASTNode &node) = 0;

  virtual void visit(const KeyParamExpressionASTNode &node) = 0;

};

}
