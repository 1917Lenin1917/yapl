//
// Created by lenin on 17.11.2024.
//

#include "yapl/ASTNode.hpp"


#include <algorithm>
#include <memory>

namespace yapl {

//
// LiteralASTNode
//
    std::string LiteralASTNode::print(size_t indent_size)
    {
        return REPEAT(indent_size*2, ' ')
               + std::format("{{ NodeType: LiteralASTNode, Type: {}, Value: {} }}",
                             ttype_to_string(token.type), token.value);
    }

    std::shared_ptr<Value> LiteralASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_LiteralASTNode(*this);
    }


//
// IdentifierASTNode
//
    std::string IdentifierASTNode::print(size_t indent_size)
    {
        return REPEAT(indent_size*2, ' ')
               + std::format("{{ NodeType: IdentifierASTNode, Value: {} }}",
                             token.value);
    }

    std::shared_ptr<Value> IdentifierASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_IdentifierASTNode(*this);
    }


//
// IndexASTNode
//
    std::string IndexASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: IndexASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "BaseExpression: \n";
        res += base_expr->print(indent_size+1) + "\n";
        res += REPEAT((indent_size+1)*2, ' ') + "IndexExpr: \n";
        res += index_expr->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> IndexASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_IndexASTNode(*this);
    }


//
// ArrayASTNode
//
    std::string ArrayASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: ArrayASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Values: \n";
        res += REPEAT((indent_size+1)*2, ' ') + "{\n";
        for (const auto& value : values)
        {
            res += value->print(indent_size+2) + ",\n";
        }
        res += REPEAT((indent_size+1)*2, ' ') + "}\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> ArrayASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_ArrayASTNode(*this);
    }


//
// VariableASTNode
//
    std::string VariableASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: VariableASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Type: " + print_token(type) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Name: " + name.value + ",\n";
        if (value)
        {
            res += REPEAT((indent_size+1)*2, ' ') + "Value: \n";
            res += value->print(indent_size+1) + "\n";
        }
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> VariableASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_VariableASTNode(*this);
    }


//
// UnaryOpASTNode
//
    std::string UnaryOpASTNode::print(size_t indent_size)
    {
        return "unary:(" + print_token(op) + " " + RHS->print(indent_size) + ")";
    }

    std::shared_ptr<Value> UnaryOpASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_UnaryOpASTNode(*this);
    }


//
// BinaryOpASTNode
//
    std::string BinaryOpASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: BinaryOPASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Operator: " + print_token(op) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "LHS: \n";
        res += LHS->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "RHS: \n";
        res += RHS->print(indent_size+1) + ",\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> BinaryOpASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_BinaryOpASTNode(*this);
    }


//
// StatementASTNode
//
    std::string StatementASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: StatementASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Identifier: " + identifier.value + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "RHS: \n";
        res += RHS->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> StatementASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_StatementASTNode(*this);
    }


//
// StatementIndexASTNode
//
    std::string StatementIndexASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: StatementIndexASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Identifier: \n";
        res += identifier->print(indent_size+1) + "\n";
        res += REPEAT((indent_size+1)*2, ' ') + "RHS: \n";
        res += RHS->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> StatementIndexASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_StatementIndexASTNode(*this);
    }


//
// FunctionArgumentASTNode
//
    std::string FunctionArgumentASTNode::print(size_t indent_size)
    {
        return REPEAT(indent_size*2, ' ')
               + std::format("{{ NodeType: FunctionArgumentASTNode, Name: {}, Type: {} }}",
                             name.value, type.value);
    }

    std::shared_ptr<Value> FunctionArgumentASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_FunctionArgumentASTNode(*this);
    }


//
// FunctionArgumentListASTNode
//
    std::string FunctionArgumentListASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: FunctionArgumentListASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Arguments: \n";
        res += REPEAT((indent_size+1)*2, ' ') + "{\n";
        for (const auto& arg : args)
        {
            res += arg->print(indent_size+2) + ",\n";
        }
        res += REPEAT((indent_size+1)*2, ' ') + "}\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> FunctionArgumentListASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_FunctionArgumentListASTNode(*this);
    }


//
// FunctionDeclASTNode
//
    std::string FunctionDeclASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: FunctionDeclASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Function Name: " + name.value + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Function Arguments: \n";
        res += args->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> FunctionDeclASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_FunctionDeclASTNode(*this);
    }


//
// MethodCallASTNode
//
    std::string MethodCallASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: MethodCallASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Identifier: \n";
        res += base_expr->print(indent_size+2) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Name: " + name.value + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Arguments: \n";
        res += REPEAT((indent_size+1)*2, ' ') + "{\n";
        for (const auto& arg : args)
            res += arg->print(indent_size+2) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "}\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> MethodCallASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_MethodCallASTNode(*this);
    }


//
// FunctionCallASTNode
//
    std::string FunctionCallASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: FunctionCallASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Name: " + name.value + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Arguments: \n";
        res += REPEAT((indent_size+1)*2, ' ') + "{\n";
        for (const auto& arg : args)
            res += arg->print(indent_size+2) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "}\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> FunctionCallASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_FunctionCallASTNode(*this);
    }


//
// ReturnStatementASTNode
//
    std::string ReturnStatementASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: ReturnStatementASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "ReturnValue: \n";
        res += expr->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> ReturnStatementASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_ReturnStatementASTNode(*this);
    }


//
// ScopeASTNode
//
    std::string ScopeASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: ScopeASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Nodes:\n";
        res += REPEAT((indent_size+1)*2, ' ') + "{\n";
        for (const auto& i : nodes)
            res += i->print(indent_size+2) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "}\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> ScopeASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_ScopeASTNode(*this);
    }


//
// FunctionASTNode
//
    std::string FunctionASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: FunctionASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "FunctionDeclaration: \n";
        res += decl->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "FunctionBody: \n";
        res += body->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> FunctionASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_FunctionASTNode(*this);
    }


//
// BuiltinCustomVisitFunctionASTNode
//
    std::string BuiltinCustomVisitFunctionASTNode::print(size_t indent_size)
    {
        return "";
    }

    std::shared_ptr<Value> BuiltinCustomVisitFunctionASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_BuiltinCustomVisitFunctionASTNode(*this);
    }

//
// IfElseExpressionASTNode
//
    std::string IfElseExpressionASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: IfElseExpressionASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Condition: \n";
        res += condition->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "TrueScope:\n";
        res += true_scope->print(indent_size+1) + ",\n";
        if (false_scope)
        {
            res += REPEAT((indent_size+1)*2, ' ') + "FalseScope:\n";
            res += false_scope->print(indent_size+1);
            res += REPEAT((indent_size+1)*2, ' ') + "\n";
        }
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> IfElseExpressionASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_IfElseExpressionASTNode(*this);
    }


//
// WhileLoopASTNode
//
    std::string WhileLoopASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: WhileLoopASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Condition: \n";
        res += condition->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Scope: \n";
        res += scope->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> WhileLoopASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_WhileLoopASTNode(*this);
    }


//
// ForLoopASTNode
//
    std::string ForLoopASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: ForLoopASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Declaration: \n";
        res += declaration->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Condition: \n";
        res += condition->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Increment: \n";
        res += increment->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Scope: \n";
        res += scope->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::shared_ptr<Value> ForLoopASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_ForLoopASTNode(*this);
    }

//
// RootASTNode
//
    std::string RootASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: RootASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Nodes:\n";
        res += REPEAT((indent_size+1)*2, ' ') + "{\n";
        for (const auto& i : nodes)
            res += i->print(indent_size+2) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "}\n";
        res += REPEAT(indent_size*2, ' ') + "}\n";
        return res;
    }

    std::shared_ptr<Value> RootASTNode::visit(Visitor &visitor)
    {
        return visitor.visit_RootASTNode(*this);
    }

    std::string StarredExpressionASTNode::print(size_t indent_size) {
        return std::string();
    }

    std::shared_ptr<Value> StarredExpressionASTNode::visit(Visitor &visitor) {
        return visitor.visit_StarredExpressionASTNode(*this);
    }
} // namespace yapl
