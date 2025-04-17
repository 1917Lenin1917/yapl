//
// Created by lenin on 17.11.2024.
//

#pragma once
#include "Interpreter.hpp"
#include "ASTNode.hpp"

#include "yapl/values/Value.hpp"

#include "memory"

namespace yapl {
class Interpreter; // forward declaration because c++ is strange
class Value;
class RootASTNode;
class LiteralASTNode;
class IdentifierASTNode;
class ArrayASTNode;
class IndexASTNode;
class VariableASTNode;
class UnaryOpASTNode;
class BinaryOpASTNode;
class StatementASTNode;
class StatementIndexASTNode;
class IfElseExpressionASTNode;
class WhileLoopASTNode;
class ForLoopASTNode;
class ReturnStatementASTNode;
class ScopeASTNode;
class FunctionArgumentASTNode;
class FunctionArgumentListASTNode;
class FunctionDeclASTNode;
class FunctionCallASTNode;
class MethodCallASTNode;
class FunctionASTNode;
class BuiltinCustomVisitFunctionASTNode;

class Visitor
{
private:
public:
  yapl::Interpreter& interpreter;
  explicit Visitor(Interpreter& i)
    :interpreter(i)
	{
		// set built-in functions
		// TODO:
	}

    std::shared_ptr<Value> visit_RootASTNode(const RootASTNode &node);
    std::shared_ptr<Value> visit_LiteralASTNode(const LiteralASTNode &node);
    std::shared_ptr<Value> visit_IdentifierASTNode(const IdentifierASTNode &node);
    std::shared_ptr<Value> visit_ArrayASTNode(const ArrayASTNode &node);
    std::shared_ptr<Value> visit_IndexASTNode(const IndexASTNode &node);
    std::shared_ptr<Value> visit_VariableASTNode(const VariableASTNode &node);
    std::shared_ptr<Value> visit_UnaryOpASTNode(const UnaryOpASTNode &node);
    std::shared_ptr<Value> visit_BinaryOpASTNode(const BinaryOpASTNode &node);
    std::shared_ptr<Value> visit_StatementASTNode(const StatementASTNode &node);
    std::shared_ptr<Value> visit_StatementIndexASTNode(const StatementIndexASTNode &node);
    std::shared_ptr<Value> visit_IfElseExpressionASTNode(const IfElseExpressionASTNode &node);
    std::shared_ptr<Value> visit_WhileLoopASTNode(const WhileLoopASTNode &node);
    std::shared_ptr<Value> visit_ForLoopASTNode(const ForLoopASTNode &node);
    std::shared_ptr<Value> visit_ReturnStatementASTNode(const ReturnStatementASTNode &node);
    std::shared_ptr<Value> visit_ScopeASTNode(const ScopeASTNode &node);
    std::shared_ptr<Value> visit_FunctionArgumentASTNode(const FunctionArgumentASTNode &node);
    std::shared_ptr<Value> visit_FunctionArgumentListASTNode(const FunctionArgumentListASTNode &node);
    std::shared_ptr<Value> visit_FunctionDeclASTNode(const FunctionDeclASTNode &node);
    std::shared_ptr<Value> visit_FunctionCallASTNode(const FunctionCallASTNode& node);
    std::shared_ptr<Value> visit_MethodCallASTNode(const MethodCallASTNode &node);
    std::shared_ptr<Value> visit_FunctionASTNode(const FunctionASTNode &node);
    std::shared_ptr<Value> visit_BuiltinCustomVisitFunctionASTNode(const BuiltinCustomVisitFunctionASTNode &node);
};
}
