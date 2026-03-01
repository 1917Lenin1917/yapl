//
// Created by lenin on 17.11.2024.
//

#include "yapl/ASTNode.hpp"


#include <algorithm>
#include <memory>

namespace yapl {

//
// IntegerASTNode
//

BaseASTNode::BaseASTNode(const std::size_t id)
    :id(id) { }

std::string IntegerASTNode::print(size_t indent_size)
{
    return REPEAT(indent_size*2, ' ')
           + std::format("{{ NodeType: IntegerASTNode, Value: {} }}",value);
}

//
// StringASTNode
//

std::string StringASTNode::print(size_t indent_size)
{
    return REPEAT(indent_size*2, ' ')
           + std::format("{{ NodeType: StringASTNode, Value: {} }}",value);
}

//
// BooleanASTNode
//

std::string BooleanASTNode::print(size_t indent_size)
{
    return REPEAT(indent_size*2, ' ')
           + std::format("{{ NodeType: BooleanASTNode, Value: {} }}",value);
}

//
// FloatASTNode
//

    std::string FloatASTNode::print(size_t indent_size)
    {
        return REPEAT(indent_size*2, ' ')
               + std::format("{{ NodeType: FloatASTNode, Value: {} }}",value);
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

    std::string DictASTNode::print(size_t indent_size)
    {
        return "TODO: DICT DEFINITION";
    }

std::string ClassASTNode::print(size_t indent_size)
{
    std::string res;
    res += REPEAT(indent_size*2, ' ') + "{\n";
    res += REPEAT((indent_size+1)*2, ' ') + "NodeType: ClassASTNode,\n";
    res += REPEAT((indent_size+1)*2, ' ') + "Name: " + name.value + ",\n";
    res += REPEAT((indent_size+1)*2, ' ') + "Methods:\n{\n";
    for(const auto& v : member_functions)
        res += v->print(indent_size+1) + ",\n";
    res += REPEAT((indent_size+1)*2, ' ') + "}\n";
    res += REPEAT(indent_size*2, ' ') + "}\n";

    return res;
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

//
// UnaryOpASTNode
//
    std::string UnaryOpASTNode::print(size_t indent_size)
    {
        return "unary:(" + print_token(op) + " " + RHS->print(indent_size) + ")";
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

//
// StatementASTNode
//
    std::string StatementASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: StatementASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Base: " + base->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "RHS: \n";
        res += RHS->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::string ImportASTNode::print(size_t indent_size)
    {
        return "import";
    }

    std::string ExportASTNode::print(size_t indent_size)
    {
        return "export";
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
//
// FunctionArgumentASTNode
//
    std::string FunctionArgumentASTNode::print(size_t indent_size)
    {
        return REPEAT(indent_size*2, ' ')
               + std::format("{{ NodeType: FunctionArgumentASTNode, Name: {}, Type: {} }}",
                             name.value, type.value);
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
    std::string GetPropertyASTNode::print(size_t indent_size)
    {
        return "TODO";
    }

    std::string SetPropertyASTNode::print(size_t indent_size)
    {
        return "TODO";
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
//
// FunctionCallASTNode
//
    std::string FunctionCallASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: FunctionCallASTNode,\n";
        // res += REPEAT((indent_size+1)*2, ' ') + "Name: " + name.value + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Base: " + base->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Arguments: \n";
        res += REPEAT((indent_size+1)*2, ' ') + "{\n";
        for (const auto& arg : args)
            res += arg->print(indent_size+2) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "}\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
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

    std::string ForEachLoopASTNode::print(size_t indent_size)
    {
        std::string res;
        res += REPEAT(indent_size*2, ' ') + "{\n";
        res += REPEAT((indent_size+1)*2, ' ') + "NodeType: ForEachLoopASTNode,\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Identifier: \n";
        res += print_token(identifier) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Iterable: \n";
        res += iterable_expr->print(indent_size+1) + ",\n";
        res += REPEAT((indent_size+1)*2, ' ') + "Scope: \n";
        res += scope->print(indent_size+1) + "\n";
        res += REPEAT(indent_size*2, ' ') + "}";
        return res;
    }

    std::string KeyParamExpressionASTNode::print(size_t indent_size)
    {
        return std::string();
    }

    std::string StarredExpressionASTNode::print(size_t indent_size) {
        return std::string();
    }
} // namespace yapl
