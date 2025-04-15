//
// Created by lenin on 17.11.2024.
//

#include "yapl/Visitor.hpp"

#include "yapl/values/Value.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/ArrayValue.hpp"

namespace yapl {

    std::shared_ptr<Value> Visitor::visit_RootASTNode(const RootASTNode &root)
    {
        for (const auto& node : root.nodes)
        {
            auto res = node->visit(*this);
            // TODO: think about proper REPL implementation
//            if (res != nullptr)
//            {
//                if (res->type == VALUE_TYPE::INTEGER)
//                    std::cout << dynamic_cast<IntegerValue*>(res.get())->value << std::endl;
//            }
        }
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_LiteralASTNode(const LiteralASTNode &node)
    {
        switch (node.token.type)
        {
            case TOKEN_TYPE::INTEGER: return std::make_unique<IntegerValue>(std::stoi(node.token.value));
            case TOKEN_TYPE::BOOL: return std::make_unique<BooleanValue>(node.token.value == std::string("true")); // This is too inefficient. Change.
            case TOKEN_TYPE::FLOAT: return std::make_unique<FloatValue>(std::stof(node.token.value));
            case TOKEN_TYPE::STRING: return std::make_unique<StringValue>(std::string(node.token.value));
            default:
            {
                // Add a proper logger
                std::cerr << "Unknown literal type!\n";
                return nullptr;
            }
        }
    }

    std::shared_ptr<Value> Visitor::visit_IdentifierASTNode(const IdentifierASTNode &node)
    {
        // TODO: Implement logic for IdentifierASTNode
        // e.g., look up variable or function name from node.token
        for (int i = interpreter.scope_stack.size() - 1; i >= 0; i--)
        {
            const auto& scope = interpreter.scope_stack[i];
            if (scope->vars.contains(node.token.value))
            {
                auto original_value = scope->vars[node.token.value]->value;
                // auto ret_value = original_value->Copy();
                // return ret_value;
                return original_value;
            }
        }
        std::cerr << std::format("Variable {} doesn't exist.\n", node.token.value);
        return nullptr;

        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_ArrayASTNode(const ArrayASTNode &node)
    {
        // TODO: Implement logic for ArrayASTNode
        // e.g., create a new array Value and visit each element
        std::vector<std::shared_ptr<Value>> ev_values;
        for (const auto& value : node.values)
        {
            ev_values.push_back(value->visit(*this));
        }
        return std::make_shared<ArrayValue>(ev_values);
    }

    std::shared_ptr<Value> Visitor::visit_IndexASTNode(const IndexASTNode &node)
    {
        auto v = node.base_expr->visit(*this)->OperatorIndex(node.index_expr->visit(*this));
        return v;
    }

    std::shared_ptr<Value> Visitor::visit_VariableASTNode(const VariableASTNode &node)
    {
        // TODO: Implement logic for VariableASTNode
        // e.g., handle variable declaration/initialization
        for (size_t i = interpreter.scope_stack.size(); i --> 0;)
        {
            auto scope = interpreter.scope_stack[i];
            if (scope->vars.contains(node.name.value))
            {
                std::cerr << std::format("Variable {} is already defined in stack idx {}/{}.\n", node.name.value, i, interpreter.scope_stack.size());
                return nullptr;
            }
        }

        auto scope = interpreter.scope_stack[interpreter.scope_stack.size() - 1];
        if (node.value)
        {
            auto expr_value = node.value->visit(*this);
            scope->vars[node.name.value] = std::make_shared<Variable>(node.type.type == TOKEN_TYPE::CONST, VALUE_TYPE::INTEGER, expr_value);
            return nullptr;
        }
        scope->vars[node.name.value] = std::make_shared<Variable>(node.type.type == TOKEN_TYPE::CONST, VALUE_TYPE::INTEGER, std::make_unique<IntegerValue>(0));
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_UnaryOpASTNode(const UnaryOpASTNode &node)
    {
        switch (node.op.type)
        {
            case TOKEN_TYPE::MINUS: { return node.RHS->visit(*this)->UnaryMinus(); }
            case TOKEN_TYPE::PLUS: { return node.RHS->visit(*this)->UnaryPlus(); }
            case TOKEN_TYPE::NOT: { return node.RHS->visit(*this)->UnaryNot(); }
            default: { std::cerr << "Unary unhandled default\n"; return nullptr; }
        }
    }

    std::shared_ptr<Value> Visitor::visit_BinaryOpASTNode(const BinaryOpASTNode &node)
    {
        const auto lhs = node.LHS->visit(*this);
        const auto rhs = node.RHS->visit(*this);
        if (node.op.type == TOKEN_TYPE::PLUS)
            return lhs->BinaryPlus(rhs);
        if (node.op.type == TOKEN_TYPE::TIMES)
            return lhs->BinaryTimes(rhs);
        if (node.op.type == TOKEN_TYPE::MINUS)
            return lhs->BinaryMinus(rhs);
        if (node.op.type == TOKEN_TYPE::SLASH)
            return lhs->BinarySlash(rhs);
        if (node.op.type == TOKEN_TYPE::LT)
            return lhs->BinaryLT(rhs);
        if (node.op.type == TOKEN_TYPE::GT)
            return lhs->BinaryGT(rhs);
        if (node.op.type == TOKEN_TYPE::LQ)
            return lhs->BinaryLQ(rhs);
        if (node.op.type == TOKEN_TYPE::GQ)
            return lhs->BinaryGQ(rhs);
        if (node.op.type == TOKEN_TYPE::EQ)
            return lhs->BinaryEQ(rhs);
        if (node.op.type == TOKEN_TYPE::MOD)
            return lhs->BinaryMOD(rhs);
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_StatementASTNode(const StatementASTNode &node)
    {
        for (size_t i = interpreter.scope_stack.size(); i --> 0 ;)
        {
            const auto& scope = interpreter.scope_stack[i];
            if (scope->vars.contains(node.identifier.value))
            {
                auto& var = scope->vars[node.identifier.value];
                if (var->is_const)
                {
                    std::cerr << std::format("Variable {} is constant.\n", node.identifier.value);
                    return nullptr;
                }
                auto rhs = node.RHS->visit(*this);
                var->value = std::move(rhs); // does this get freed automatically?
                return nullptr;
            }
        }

        std::cerr << std::format("Variable {} doesn't exist.\n", node.identifier.value);
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_StatementIndexASTNode(const StatementIndexASTNode &node)
    {
        auto id = dynamic_cast<IndexASTNode*>(node.identifier.get());

        auto lhs = id->base_expr->visit(*this);
        auto idx = id->index_expr->visit(*this);
        auto rhs = node.RHS->visit(*this);
        lhs->OperatorIndexSet(idx, rhs);
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_IfElseExpressionASTNode(const IfElseExpressionASTNode &node)
    {
        // FIXME: this really should be a macro|helper function
        if (dynamic_cast<BooleanValue*>(node.condition->visit(*this)->BinaryEQ(std::make_unique<BooleanValue>(true)).get())->value)
            // TODO: the problem is if condition is not boolean. Probably should add method to all values to check if its truthy or falsy
//        auto condition_eval = node.condition->visit(*this);
//        if (dynamic_cast<BooleanValue*>(condition_eval.get())->value)
        {
            interpreter.push_scope();
            auto res = node.true_scope->visit(*this);
            interpreter.pop_scope();
            return res;
        }
        if (node.false_scope)
        {
            interpreter.push_scope();
            auto res = node.false_scope->visit(*this);
            interpreter.pop_scope();
            return res;
        }
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_WhileLoopASTNode(const WhileLoopASTNode &node)
    {
        while (dynamic_cast<BooleanValue*>(node.condition->visit(*this)->BinaryEQ(std::make_unique<BooleanValue>(true)).get())->value == true)
        {
            interpreter.push_scope();
            node.scope->visit(*this);
            interpreter.pop_scope();
        }
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_ForLoopASTNode(const ForLoopASTNode &node)
    {
        interpreter.push_scope();
        node.declaration->visit(*this);
        while (dynamic_cast<BooleanValue*>(node.condition->visit(*this)->BinaryEQ(std::make_unique<BooleanValue>(true)).get())->value == true)
        {
            interpreter.push_scope();
            node.scope->visit(*this);
            interpreter.pop_scope();
            node.increment->visit(*this);
        }
        interpreter.pop_scope();
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_ReturnStatementASTNode(const ReturnStatementASTNode &node)
    {
        auto func = interpreter.function_stack[interpreter.function_stack.size() - 1];
        auto rhs = node.expr->visit(*this);
        func->return_value = rhs;
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_ScopeASTNode(const ScopeASTNode &node)
    {
        const auto current_func = interpreter.function_stack.empty() ? nullptr : interpreter.function_stack[interpreter.function_stack.size() - 1];
        for (const auto& i : node.nodes)
        {
            i->visit(*this);

            if (current_func != nullptr && current_func->return_value != nullptr)
                return nullptr;
        }
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_FunctionArgumentASTNode(const FunctionArgumentASTNode &node)
    {
        // TODO: Implement logic for FunctionArgumentASTNode
        // e.g., track argument name/type for function definitions
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_FunctionArgumentListASTNode(const FunctionArgumentListASTNode &node)
    {
        // TODO: Implement logic for FunctionArgumentListASTNode
        // e.g., handle multiple arguments from node.args
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_FunctionDeclASTNode(const FunctionDeclASTNode &node)
    {
        // TODO: Implement logic for FunctionDeclASTNode
        // e.g., register function signature (node.name, node.args, node.return_type)
        return nullptr;
    }

    std::shared_ptr<Value> Visitor::visit_FunctionCallASTNode(const FunctionCallASTNode &node)
    {
        if (!interpreter.function_exists(node.name.value))
        {
            std::cout << "Function doesn't exist\n";
            return nullptr;
        }

        auto func_def = interpreter.get_function_def(node.name.value);
        auto func = interpreter.push_function(node.name.value);
        auto arg_list = dynamic_cast<FunctionArgumentListASTNode*>(dynamic_cast<FunctionDeclASTNode*>(func_def->decl.get())->args.get());
        size_t arg_amount = arg_list->get_argument_amount();
        if (arg_amount != -1 && node.args.size() != arg_amount)
        {
            std::cerr << std::format("Invalid amount of arguments provided to function {}. Expected {} arguments, instead got {}.\n", node.name.value, arg_amount, node.args.size());
            return nullptr;
        }
        // set args with passed expressions
        for (size_t i = 0; i < node.args.size(); i++)
        {
            auto value = node.args[i]->visit(*this);
            func->set_argument(std::make_unique<Variable>(false, value->type, std::move(value)), arg_list->get_argument_name(i));
            // func->function_scope->vars[arg_list->args[i].get()->name.value] = std::make_unique<Variable>(false, value->type, std::move(value));
        }
        func_def->body->visit(*this);
        auto ret_value = func->return_value;
        interpreter.pop_scope();
        interpreter.pop_function();

        return ret_value;
    }

    std::shared_ptr<Value> Visitor::visit_MethodCallASTNode(const MethodCallASTNode &node)
    {
        // FIXME: copy LHS logic from indexing operator, so you can call method of stuff like (1+2).to_string()
//        auto object = interpreter.get_variable(node.identifier.value);
//        if (!object)
//        {
//            std::cerr << "Object " << node.identifier.value << " doesn't exist\n";
//            return nullptr;
//        }

        auto object = node.base_expr->visit(*this);
        auto func_def = object->get_method_definition(node.name.value);
        auto func = interpreter.push_function(std::format("[{}].{}", static_cast<const void*>(object.get()), node.name.value)); // maybe get this from value ?
        auto arg_list = dynamic_cast<FunctionArgumentListASTNode*>(dynamic_cast<FunctionDeclASTNode*>(func_def->decl.get())->args.get());
        size_t arg_amount = arg_list->get_argument_amount();
        if (arg_amount != -1 && node.args.size() != arg_amount)
        {
            std::cerr << std::format("Invalid amount of arguments provided to function {}. Expected {} arguments, instead got {}.\n", node.name.value, arg_amount, node.args.size());
            return nullptr;
        }
        // set args with passed expressions
        for (size_t i = 0; i < node.args.size(); i++)
        {
            auto value = node.args[i]->visit(*this);
            func->set_argument(std::make_unique<Variable>(false, value->type, std::move(value)), arg_list->get_argument_name(i));
            // func->function_scope->vars[arg_list->args[i].get()->name.value] = std::make_unique<Variable>(false, value->type, std::move(value));
        }
        func_def->body->visit(*this);
        auto ret_value = func->return_value;
        interpreter.pop_scope();
        interpreter.pop_function();

        return ret_value;
    }

    std::shared_ptr<Value> Visitor::visit_FunctionASTNode(const FunctionASTNode &node)
    {
        const auto& f_decl = dynamic_cast<FunctionDeclASTNode*>(node.decl.get());
        if (interpreter.function_exists(f_decl->name.value))
        {
            std::cout << "Function " << f_decl->name.value << " already exists!\n";
            return nullptr;
        }

        interpreter.add_function_definition(f_decl->name.value, const_cast<FunctionASTNode *>(&node));

        // TODO: maybe add a function type, so functions can be passed like objects or lambdas
        return nullptr; // should there be a function type??
    }

    std::shared_ptr<Value> Visitor::visit_BuiltinCustomVisitFunctionASTNode(const BuiltinCustomVisitFunctionASTNode &node)
    {
        auto func_obj = interpreter.function_stack[interpreter.function_stack.size() - 1];
        func_obj->return_value = node.func(func_obj);
        return nullptr;
    }
}