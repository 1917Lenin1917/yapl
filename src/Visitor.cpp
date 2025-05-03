//
// Created by lenin on 17.11.2024.
//

#include <format>

#include "yapl/Visitor.hpp"

#include "yapl/values/Value.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/ArrayValue.hpp"
#include "yapl/exceptions/RuntimeError.hpp"

namespace yapl {

    std::shared_ptr<Value> Visitor::visit_RootASTNode(const RootASTNode &root)
    {
        std::shared_ptr<Value> return_value = nullptr;
        for (const auto& node : root.nodes)
        {
            auto res = node->visit(*this);
            return_value = res;
            // TODO: think about proper REPL implementation
        }
        return return_value;
    }

    std::shared_ptr<Value> Visitor::visit_LiteralASTNode(const LiteralASTNode &node)
    {
        throw RuntimeError("visit_LiteralASTNode is deprecated!");
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

    std::shared_ptr<Value> Visitor::visit_IntegerASTNode(const IntegerASTNode &node)
    {
        return mk_int(node.value);
    }
    std::shared_ptr<Value> Visitor::visit_StringASTNode(const StringASTNode &node)
    {
        return mk_str(node.value);
    }
    std::shared_ptr<Value> Visitor::visit_FloatASTNode(const FloatASTNode &node)
    {
        return mk_float(node.value);
    }
    std::shared_ptr<Value> Visitor::visit_BooleanASTNode(const BooleanASTNode &node)
    {
        return mk_bool(node.value);
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
        VPtr ret_val;
        auto value = node.RHS->visit(*this);
        switch (node.op.type)
        {
            case TOKEN_TYPE::MINUS: { ret_val = value->UnaryMinus(); break; }
            case TOKEN_TYPE::PLUS: { ret_val = value->UnaryPlus(); break; }
            case TOKEN_TYPE::NOT: { ret_val = value->UnaryNot(); break; }
            default: { std::cerr << "Unary unhandled default\n"; return nullptr; }
        }
        if (ret_val == NotImplemented)
            throw yapl::RuntimeError(std::format("Unsupported operand type for unary {}: '{}'", ttype_to_string(node.op.type), value_type_to_string(value->type)));

        return ret_val;
    }

    std::shared_ptr<Value> Visitor::visit_BinaryOpASTNode(const BinaryOpASTNode &node)
    {
        const auto lhs = node.LHS->visit(*this);
        const auto rhs = node.RHS->visit(*this);

        binop_fn op, rop;

        if (node.op.type == TOKEN_TYPE::PLUS)
        {
            op  = lhs->tp->nb_add;
            rop = rhs->tp->nb_add;
        }
        if (node.op.type == TOKEN_TYPE::TIMES)
        {
            op  = lhs->tp->nb_mul;
            rop = rhs->tp->nb_mul;
        }
        if (node.op.type == TOKEN_TYPE::MINUS)
        {
            op  = lhs->tp->nb_sub;
            rop = rhs->tp->nb_sub;
        }
        if (node.op.type == TOKEN_TYPE::SLASH)
        {
            op  = lhs->tp->nb_div;
            rop = rhs->tp->nb_div;
        }
        if (node.op.type == TOKEN_TYPE::MOD)
        {
            op  = lhs->tp->nb_mod;
            rop = rhs->tp->nb_mod;
        }
        if (node.op.type == TOKEN_TYPE::LT)
        {
            op  = lhs->tp->nb_lt;
            rop = rhs->tp->nb_lt;
        }
        if (node.op.type == TOKEN_TYPE::GT)
        {
            op  = lhs->tp->nb_gt;
            rop = rhs->tp->nb_gt;
        }
        if (node.op.type == TOKEN_TYPE::LQ)
        {
            op  = lhs->tp->nb_le;
            rop = rhs->tp->nb_le;
        }
        if (node.op.type == TOKEN_TYPE::GQ)
        {
            op  = lhs->tp->nb_ge;
            rop = rhs->tp->nb_ge;
        }
        if (node.op.type == TOKEN_TYPE::EQ)
        {
            op  = lhs->tp->nb_eq;
            rop = rhs->tp->nb_eq;
        }
        // TODO: add NEQ!!!

        if (op) {
            VPtr r = op(lhs, rhs);
            if (r != NotImplemented) return r;
        }
        if (rop) {            // reverse call
            VPtr r = rop(rhs, lhs);
            if (r != NotImplemented) return r;
        }
        throw yapl::RuntimeError(std::format("Unsupported operand types for {}: '{}' and '{}'", ttype_to_string(node.op.type), value_type_to_string(lhs->type), value_type_to_string(rhs->type)));
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
        if (node.condition->visit(*this)->IsTruthy())
        {
            interpreter.push_scope();
            auto res = node.true_scope->visit(*this);
            interpreter.pop_scope();
            return res;
        }
        else if (node.false_scope)
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
        while(node.condition->visit(*this)->IsTruthy())
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
        while (node.condition->visit(*this)->IsTruthy())
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
        // check if we have variadic args
        // TODO: handle kwargs
        if (const auto& args_arg = arg_list->args_arg)
        {
            auto list = std::vector<VPtr>{  };
            for (const auto& arg : node.args)
            {
                // TODO: this dynamic_cast is probably really slow!
                if (const auto& str_expr = dynamic_cast<StarredExpressionASTNode*>(arg.get()))
                {
                    auto v = str_expr->visit(*this);
                    for (const auto& val : as_arr(v.get())->value)
                    {
                        list.push_back(val);
                    }
                    continue;
                }
                list.push_back(arg->visit(*this));
            }
            auto VPtrList = std::make_unique<ArrayValue>(list);
            func->set_argument(std::make_unique<Variable>(false, VPtrList->type, std::move(VPtrList)), arg_list->args_arg->name.value);
        }
        else
        {
            // TODO: add starred expression handling
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
        }
        func_def->body->visit(*this);
        auto ret_value = func->return_value;
        interpreter.pop_scope();
        interpreter.pop_function();

        return ret_value;
    }

    std::shared_ptr<Value> Visitor::visit_MethodCallASTNode(const MethodCallASTNode &node)
    {
        auto object = node.base_expr->visit(*this);
        auto func_def = object->get_method_definition(node.name.value);
        auto func = interpreter.push_function(std::format("[{}].{}", static_cast<const void*>(object.get()), node.name.value)); // maybe get this from value ?
        auto arg_list = dynamic_cast<FunctionArgumentListASTNode*>(dynamic_cast<FunctionDeclASTNode*>(func_def->decl.get())->args.get());


        // pass object reference to the method as the first argument
        func->set_argument(std::make_unique<Variable>(false, object->type, object), "this");

        // check if we have variadic args
        // TODO: handle kwargs
        if (const auto& args_arg = arg_list->args_arg)
        {
            auto list = std::vector<VPtr>{  };
            for (const auto& arg : node.args)
            {
                // TODO: this dynamic_cast is probably really slow!
                if (const auto& str_expr = dynamic_cast<StarredExpressionASTNode*>(arg.get()))
                {
                    auto v = str_expr->visit(*this);
                    for (const auto& val : as_arr(v.get())->value)
                    {
                        list.push_back(val);
                    }
                    continue;
                }
                list.push_back(arg->visit(*this));
            }
            auto VPtrList = std::make_unique<ArrayValue>(list);
            func->set_argument(std::make_unique<Variable>(false, VPtrList->type, std::move(VPtrList)), arg_list->args_arg->name.value);
        }
        else
        {
            size_t arg_amount = arg_list->get_argument_amount();
            if (arg_amount != -1 && node.args.size() + 1 != arg_amount)
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

    std::shared_ptr<Value> Visitor::visit_StarredExpressionASTNode(const StarredExpressionASTNode &node) {
        auto val = node.expression->visit(*this);
        // TODO: add proper check for iterable
        // for now we'll just work with arrays only
        if (val->tp != ArrayTypeObject)
            throw RuntimeError("Starred expression expected an array object.");
        return val;
    }
}