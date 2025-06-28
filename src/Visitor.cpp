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
#include "yapl/values/DictValue.hpp"
#include "yapl/values/TypeObjectValue.hpp"
#include "yapl/values/UserDefinedValue.hpp"

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
    if (const auto var = interpreter.GetVariable(node.token.value))
    {
        return var->value;
    }
    throw RuntimeError(std::format("Variable '{}' doesn't exist.", node.token.value));
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

std::shared_ptr<Value> Visitor::visit_DictASTNode(const DictASTNode &node)
{
    auto dict = mk_dict();
    assert(node.keys.size() == node.values.size());

    const size_t len = node.keys.size();
    for (size_t i = 0; i < len; i++)
    {
        dict->OperatorIndexSet(node.keys[i]->visit(*this), node.values[i]->visit(*this));
    }

    return dict;
}


std::shared_ptr<Value> Visitor::visit_IndexASTNode(const IndexASTNode &node)
{
    auto v = node.base_expr->visit(*this)->OperatorIndex(node.index_expr->visit(*this));
    return v;
}

std::shared_ptr<Value> Visitor::visit_VariableASTNode(const VariableASTNode &node)
{
    if (auto var = interpreter.GetVariable(node.name.value))
    {
       throw RuntimeError(std::format("Variable {} is already defined.", node.name.value));
    }

    interpreter.AddVariable(node.name.value, node.type.type == TOKEN_TYPE::CONST, node.value ? node.value->visit(*this) : mk_int(0));

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
    auto base = node.base->visit(*this);
    auto RHS = node.RHS->visit(*this);
    base->Set(RHS);

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
    auto parent_scope = interpreter.scope_stack[interpreter.scope_stack.size() - 1];
    if (node.condition->visit(*this)->IsTruthy())
    {
        interpreter.push_scope();
        interpreter.scope_stack[interpreter.scope_stack.size() - 1]->parent_scope = parent_scope;
        auto res = node.true_scope->visit(*this);
        interpreter.pop_scope();
        return res;
    }
    if (node.false_scope)
    {
        interpreter.push_scope();
        interpreter.scope_stack[interpreter.scope_stack.size() - 1]->parent_scope = parent_scope;
        auto res = node.false_scope->visit(*this);
        interpreter.pop_scope();
        return res;
    }
    return nullptr;
}

std::shared_ptr<Value> Visitor::visit_WhileLoopASTNode(const WhileLoopASTNode &node)
{
    auto parent_scope = interpreter.scope_stack[interpreter.scope_stack.size() - 1];
    while(node.condition->visit(*this)->IsTruthy())
    {
        interpreter.push_scope();
        interpreter.scope_stack[interpreter.scope_stack.size() - 1]->parent_scope = parent_scope;
        node.scope->visit(*this);
        interpreter.pop_scope();
    }
    return nullptr;
}

std::shared_ptr<Value> Visitor::visit_ForLoopASTNode(const ForLoopASTNode &node)
{
    auto parent_scope = interpreter.scope_stack[interpreter.scope_stack.size() - 1];
    interpreter.push_scope();
    interpreter.scope_stack[interpreter.scope_stack.size() - 1]->parent_scope = parent_scope;
    auto for_scope = interpreter.scope_stack[interpreter.scope_stack.size() - 1];
    node.declaration->visit(*this);
    while (node.condition->visit(*this)->IsTruthy())
    {
        interpreter.push_scope();
        interpreter.scope_stack[interpreter.scope_stack.size() - 1]->parent_scope = for_scope;
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
    // if (!interpreter.function_exists(node.name.value))
    // {
    //     std::cout << "Function doesn't exist\n";
    //     return nullptr;
    // }
    auto base = node.base->visit(*this);
    return base->tp->nb_call(*this, base, node.args);

}

std::shared_ptr<Value> Visitor::visit_MethodCallASTNode(const MethodCallASTNode &node)
{
    auto object = node.base_expr->visit(*this);
    auto func_def = object->get_method_definition(node.name.value);
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

        auto func = interpreter.push_function(std::format("[{}].{}", static_cast<const void*>(object.get()), node.name.value)); // maybe get this from value ?
        // pass object reference to the method as the first argument
        func->set_argument(std::make_unique<Variable>(false, object->type, object), "this");
        func->set_argument(std::make_unique<Variable>(false, VPtrList->type, std::move(VPtrList)), arg_list->args_arg->name.value);
        func_def->body->visit(*this);
        auto ret_value = func->return_value;
        interpreter.pop_scope();
        interpreter.pop_function();

        return ret_value;
    }
    size_t arg_amount = arg_list->get_argument_amount();
    if (arg_amount != -1 && node.args.size() + 1 != arg_amount)
    {
        std::cerr << std::format("Invalid amount of arguments provided to function {}. Expected {} arguments, instead got {}.\n", node.name.value, arg_amount, node.args.size());
        return nullptr;
    }

    std::vector<VPtr> values;
    // set args with passed expressions
    for (const auto & arg : node.args)
    {
        auto value = arg->visit(*this);
        values.push_back(value);
    }
    auto func = interpreter.push_function(std::format("[{}].{}", static_cast<const void*>(object.get()), node.name.value)); // maybe get this from value ?
    // pass object reference to the method as the first argument
    func->set_argument(std::make_unique<Variable>(false, object->type, object), "this");
    for (int i = 0; i < values.size(); i++)
    {
        auto value = values[i];
        func->set_argument(std::make_unique<Variable>(false, value->type, std::move(value)), arg_list->get_argument_name(i+1)); // skip first arg (this)
    }
    func_def->body->visit(*this);
    auto ret_value = func->return_value;
    interpreter.pop_scope();
    interpreter.pop_function();

    return ret_value;
}

std::shared_ptr<Value> Visitor::visit_GetPropertyASTNode(const GetPropertyASTNode &node)
{
    auto obj = node.base_expr->visit(*this);
    return obj->GetField(node.name.value);
}

std::shared_ptr<Value> Visitor::visit_SetPropertyASTNode(const SetPropertyASTNode &node)
{
    auto obj = node.base_expr->visit(*this);
    auto RHS = node.RHS->visit(*this);

    return obj->SetField(node.name.value, RHS);
}

std::shared_ptr<Value> Visitor::visit_FunctionASTNode(const FunctionASTNode &node)
{
    const auto& f_decl = dynamic_cast<FunctionDeclASTNode*>(node.decl.get());
    if (interpreter.function_exists(f_decl->name.value))
    {
        throw RuntimeError(std::format("Function {} already exists.", f_decl->name.value));
    }

    // FIXME: create a copy of FunctionASTNode maybe?
    auto fn = interpreter.add_function_definition(f_decl->name.value, const_cast<FunctionASTNode *>(&node));
    interpreter.scope_stack[0]->vars[f_decl->name.value] = std::make_shared<Variable>(true, VALUE_TYPE::FUNCTION, fn);

    // TODO: maybe add a function type, so functions can be passed like objects or lambdas
    return fn;
}

std::shared_ptr<Value> Visitor::visit_ClassASTNode(const ClassASTNode &node)
{
    auto UserDefinedTypeObject = new TypeObject{ node.name.value };
    auto new_type = mk_type(UserDefinedTypeObject);

    interpreter.types[new_type->value->name] = new_type;
    auto scope = interpreter.scope_stack[0];
    scope->vars[new_type->value->name] = std::make_unique<Variable>(true, VALUE_TYPE::TYPE, new_type);

    for (const auto& v : node.member_functions)
    {
        auto fn = static_cast<FunctionASTNode*>(v.get());
        auto fn_decl = static_cast<FunctionDeclASTNode*>(fn->decl.get());

        new_type->value->AddMethod(fn_decl->name.value, std::unique_ptr<FunctionASTNode>(fn)); // FIXME: this is a bug
    }

    // constructor is required
    new_type->value->nb_make = [UserDefinedTypeObject, this](const std::vector<VPtr>& args) -> VPtr
    {
        auto new_obj = std::make_shared<UserDefinedValue>(UserDefinedTypeObject);
        auto init_args = std::vector<std::unique_ptr<BaseASTNode>>{};
        init_args.reserve(args.size());
        for (const auto& arg : args)
            init_args.push_back(std::make_unique<InternalGetValueASTNode>(arg));
        const auto method = std::make_unique<MethodCallASTNode>(std::make_unique<InternalGetValueASTNode>(new_obj), Token{TOKEN_TYPE::IDENTIFIER, new char[]{"init"}}, init_args);
        visit_MethodCallASTNode(*method);
        return new_obj;
    };
    new_type->value->nb_str = [this](const VPtr& self) -> VPtr
    {
        if (self->tp->method_dict.contains("__str__"))
        {
            std::vector<std::unique_ptr<BaseASTNode>> args{};
            const auto method = std::make_unique<MethodCallASTNode>(std::make_unique<InternalGetValueASTNode>(self), Token{TOKEN_TYPE::IDENTIFIER, new char[]{"__str__"}}, args);
            auto v = visit_MethodCallASTNode(*method);
            return v;
        }
        auto v = std::format("<object of type '{}' at {}>", self->tp->name, static_cast<const void*>(self.get()));
        return mk_str(v);
    };

    return nullptr;
}

std::shared_ptr<Value> Visitor::visit_BuiltinCustomVisitFunctionASTNode(const BuiltinCustomVisitFunctionASTNode &node)
{
    const auto func_obj = interpreter.GetCurrentFunction();
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