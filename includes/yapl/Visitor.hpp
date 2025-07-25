//
// Created by lenin on 17.11.2024.
//

#pragma once
#include "Interpreter.hpp"
#include "ASTNode.hpp"

#include "yapl/values/Value.hpp"

#include "memory"

#ifdef __linux__
#define DEBUG std::raise(SIGINT);
#endif
#ifdef __APPLE__
#define DEBUG std::raise(SIGINT);
#endif
#ifdef __MINGW32__
#define DEBUG __debugbreak();
#endif

namespace yapl {
class Interpreter; // forward declaration because c++ is strange
class Value;
class RootASTNode;
class LiteralASTNode;
class IntegerASTNode;
class FloatASTNode;
class StringASTNode;
class BooleanASTNode;
class IdentifierASTNode;
class ArrayASTNode;
class DictASTNode;
class IndexASTNode;
class VariableASTNode;
class UnaryOpASTNode;
class BinaryOpASTNode;
class StatementASTNode;
class StatementIndexASTNode;
class IfElseExpressionASTNode;
class WhileLoopASTNode;
class ForLoopASTNode;
class ForEachLoopASTNode;
class ReturnStatementASTNode;
class ScopeASTNode;
class FunctionArgumentASTNode;
class FunctionArgumentListASTNode;
class FunctionDeclASTNode;
class FunctionCallASTNode;
class MethodCallASTNode;
class StarredExpressionASTNode;
class FunctionASTNode;
class BuiltinCustomVisitFunctionASTNode;
class ClassASTNode;
class GetPropertyASTNode;
class SetPropertyASTNode;
class ImportASTNode;
class ExportASTNode;

class Visitor
{
public:
  Interpreter& interpreter;
  explicit Visitor(Interpreter& i)
    :interpreter(i) {}

  std::shared_ptr<Value> visit_RootASTNode(const RootASTNode &node);
  std::shared_ptr<Value> visit_LiteralASTNode(const LiteralASTNode &node);
  std::shared_ptr<Value> visit_IntegerASTNode(const IntegerASTNode &node);
  std::shared_ptr<Value> visit_FloatASTNode(const FloatASTNode &node);
  std::shared_ptr<Value> visit_BooleanASTNode(const BooleanASTNode &node);
  std::shared_ptr<Value> visit_StringASTNode(const StringASTNode &node);
  std::shared_ptr<Value> visit_IdentifierASTNode(const IdentifierASTNode &node);
  std::shared_ptr<Value> visit_ArrayASTNode(const ArrayASTNode &node);
  std::shared_ptr<Value> visit_DictASTNode(const DictASTNode &node);
  std::shared_ptr<Value> visit_IndexASTNode(const IndexASTNode &node);
  std::shared_ptr<Value> visit_VariableASTNode(const VariableASTNode &node);
  std::shared_ptr<Value> visit_UnaryOpASTNode(const UnaryOpASTNode &node);
  std::shared_ptr<Value> visit_BinaryOpASTNode(const BinaryOpASTNode &node);
  std::shared_ptr<Value> visit_StatementASTNode(const StatementASTNode &node);
  std::shared_ptr<Value> visit_ImportASTNode(const ImportASTNode &node);
  std::shared_ptr<Value> visit_ExportASTNode(const ExportASTNode &node);
  std::shared_ptr<Value> visit_StatementIndexASTNode(const StatementIndexASTNode &node);
  std::shared_ptr<Value> visit_IfElseExpressionASTNode(const IfElseExpressionASTNode &node);
  std::shared_ptr<Value> visit_WhileLoopASTNode(const WhileLoopASTNode &node);
  std::shared_ptr<Value> visit_ForLoopASTNode(const ForLoopASTNode &node);
  std::shared_ptr<Value> visit_ForEachLoopASTNode(const ForEachLoopASTNode &node);
  std::shared_ptr<Value> visit_ReturnStatementASTNode(const ReturnStatementASTNode &node);
  std::shared_ptr<Value> visit_ScopeASTNode(const ScopeASTNode &node);
  std::shared_ptr<Value> visit_FunctionArgumentASTNode(const FunctionArgumentASTNode &node);
  std::shared_ptr<Value> visit_FunctionArgumentListASTNode(const FunctionArgumentListASTNode &node);
  std::shared_ptr<Value> visit_FunctionDeclASTNode(const FunctionDeclASTNode &node);
  std::shared_ptr<Value> visit_FunctionCallASTNode(const FunctionCallASTNode& node);
  std::shared_ptr<Value> visit_MethodCallASTNode(const MethodCallASTNode &node);
  std::shared_ptr<Value> visit_GetPropertyASTNode(const GetPropertyASTNode &node);
  std::shared_ptr<Value> visit_SetPropertyASTNode(const SetPropertyASTNode &node);
  std::shared_ptr<Value> visit_StarredExpressionASTNode(const StarredExpressionASTNode &node);
  std::shared_ptr<Value> visit_FunctionASTNode(const FunctionASTNode &node);
  std::shared_ptr<Value> visit_ClassASTNode(const ClassASTNode &node);
  std::shared_ptr<Value> visit_BuiltinCustomVisitFunctionASTNode(const BuiltinCustomVisitFunctionASTNode &node);
};

}
