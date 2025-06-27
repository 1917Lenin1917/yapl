//
// Created by lenin on 15.11.2024.
//

#include <memory>
#include <sstream>

#include "yapl/Interpreter.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/TypeObjectValue.hpp"
#include <yapl/values/DictValue.hpp>
#include <yapl/values/FunctionValue.hpp>


namespace yapl {

#define MAKE_TOKEN(token_name) Token{ TOKEN_TYPE::IDENTIFIER, new char[](token_name)  }
#define MAKE_BODY(body_fn) std::make_unique<BuiltinCustomVisitFunctionASTNode>([&](std::shared_ptr<Function> f_obj) body_fn )

Interpreter::Interpreter()
{
    scope_stack.push_back(std::make_unique<Scope>()); // make global scope

    MAKE_METHOD_WITH_VARGS(this, "print", "any")
    {
        auto args = static_cast<ArrayValue*>(f_obj->function_scope->vars.at("args")->value.get());
        for (const auto& arg : args->value)
        {
            std::cout << arg->print() << " ";
        }
        std::cout << "\n";
        return nullptr;
    };

    MAKE_METHOD(this, "read_int", "int")
    {
        int value; std::cin >> value;
        return std::make_shared<IntegerValue>(value);
    };

    MAKE_METHOD(this, "read_str", "str")
    {
        std::string value; std::cin >> value;
        return std::make_shared<StringValue>(value);
    };

    MAKE_METHOD(this, "id", "any", ARG("obj", "any"))
    {
        auto arg = f_obj->function_scope->vars["obj"];
        std::stringstream ss;
        ss << static_cast<void*>(arg->value.get());
        return mk_str(ss.str());
    };

    init_int_tp();
    init_float_type();
    init_array_tp();
    init_bool_tp();
    init_str_tp();
    init_tp_tp();
    init_dict_tp();
    init_function_tp();


    types[IntegerTypeObject->name] = mk_type(IntegerTypeObject);
    types[FloatTypeObject->name] = mk_type(FloatTypeObject);
    types[ArrayTypeObject->name] = mk_type(ArrayTypeObject);
    types[BooleanTypeObject->name] = mk_type(BooleanTypeObject);
    types[StringTypeObject->name] = mk_type(StringTypeObject);
    types[TypeObjectTypeObject->name] = mk_type(TypeObjectTypeObject);
    types[DictTypeObject->name] = mk_type(DictTypeObject);
    types[FunctionTypeObject->name] = mk_type(FunctionTypeObject);

    auto scope = scope_stack[0];
    for (const auto& [first, second] : types)
    {
        scope->vars[first] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE, second);
    }
    for (const auto& [first, second] : function_definitions)
    {
        scope->vars[first] = std::make_shared<Variable>(true, VALUE_TYPE::FUNCTION, mk_func(first, second));
    }
}

bool Interpreter::function_exists(const std::string &name) const
{
  return function_definitions.contains(name);
}

std::shared_ptr<Variable> Interpreter::get_variable(const std::string &name) const
{
  for (size_t i = scope_stack.size(); i --> 0 ;)
      if (const auto scope = scope_stack[i]; scope->vars.contains(name))
          return scope->vars[name];
  return nullptr;
}
std::shared_ptr<Value> Interpreter::add_function_definition(const std::string &name, FunctionASTNode *fn)
{
    function_definitions[name] = fn;
    auto global_scope = scope_stack[0];

    return mk_func(name, fn);
}

std::shared_ptr<Function> Interpreter::push_function(const std::string& name)
{
    auto func = std::make_shared<Function>();
    func->name = name;
    function_stack.push_back(func);
    scope_stack.push_back(func->function_scope);

    return func;
}

std::shared_ptr<Variable> Interpreter::AddVariable(const std::string &name, bool is_const, VPtr value)
{
    auto scope = scope_stack[scope_stack.size() - 1];
    scope->vars[name] = std::make_shared<Variable>(is_const, value->type, value);
    return scope->vars[name];
}

/*
 * When calling this method, we assume that we're at the top of the call stack. So we can only see last stack's and its parent's vars as well as global vars
 */
std::shared_ptr<Variable> Interpreter::GetVariable(const std::string &name)
{
    // look through newest scope and its parents
    auto scope = scope_stack[scope_stack.size() - 1];
    while (scope)
    {
        if (scope->vars.contains(name))
        {
            return scope->vars.at(name);
        }
        scope = scope->parent_scope;
    }

    // look in globals
    if (const auto global_stack = scope_stack[0]; global_stack->vars.contains(name))
    {
        return global_stack->vars.at(name);
    }

    return nullptr;

}

std::shared_ptr<Function> Interpreter::GetCurrentFunction()
{
    return function_stack[function_stack.size() - 1];
}

void Interpreter::pop_function()
{
    function_stack.pop_back();
}
void Interpreter::push_scope()
{
    scope_stack.push_back(std::make_shared<Scope>());
}
void Interpreter::pop_scope()
{
    scope_stack.pop_back();
}
FunctionASTNode *Interpreter::get_function_def(const std::string &name)
{
    return function_definitions[name];
}


}
