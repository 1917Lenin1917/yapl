//
// Created by lenin on 15.11.2024.
//

#include "yapl/Interpreter.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/TypeObjectValue.hpp"
#include <memory>

namespace yapl {

#define MAKE_TOKEN(token_name) Token{ TOKEN_TYPE::IDENTIFIER, new char[](token_name)  }
#define MAKE_BODY(body_fn) std::make_unique<BuiltinCustomVisitFunctionASTNode>([&](std::shared_ptr<Function> f_obj) body_fn )

void Interpreter::make_builtin_print()
{
    auto name = MAKE_TOKEN("print");
    auto return_type = MAKE_TOKEN("void");
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
    auto body = MAKE_BODY(
    {
        auto args = static_cast<ArrayValue*>(f_obj->function_scope->vars.at("args")->value.get());
        for (const auto& arg : args->value)
        {
            std::cout << arg->print() << " ";
        }
        std::cout << "\n";
        return nullptr;
    });
    auto args_arg = std::make_unique<FunctionArgumentASTNode>(Token{TOKEN_TYPE::IDENTIFIER, new char[]{"args"}}, Token{TOKEN_TYPE::IDENTIFIER, new char[]{"any"}}, true);
    auto f = std::make_unique<FunctionASTNode>(
            std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list, std::move(args_arg),
                                                                                                                nullptr), return_type)),
            std::move(body));
    builtin_functions.push_back(std::move(f));
    function_definitions["print"] = builtin_functions[builtin_functions.size() - 1].get();
}
void Interpreter::make_builtin_read_int()
{
    auto name = MAKE_TOKEN("read_int");
    auto return_type = MAKE_TOKEN("int");
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
    auto body = MAKE_BODY(
            {
                int value; std::cin >> value;
                f_obj->return_value = std::make_shared<IntegerValue>(value);
                return nullptr;
            });
    auto f = std::make_unique<FunctionASTNode>(
            std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list), return_type)),
            std::move(body));
    builtin_functions.push_back(std::move(f));
    function_definitions["read_int"] = builtin_functions[builtin_functions.size() - 1].get();
}
void Interpreter::make_builtin_read_string()
{
    auto name = MAKE_TOKEN("read_str");
    auto return_type = MAKE_TOKEN("str");
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
    auto body = MAKE_BODY(
            {
                std::string value; std::cin >> value;
                f_obj->return_value = std::make_shared<StringValue>(value);
                return nullptr;
            });
    auto f = std::make_unique<FunctionASTNode>(
            std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list), return_type)),
            std::move(body));
    builtin_functions.push_back(std::move(f));
    function_definitions["read_str"] = builtin_functions[builtin_functions.size() - 1].get();
}

Interpreter::Interpreter()
{
    scope_stack.push_back(std::make_unique<Scope>()); // make global scope

    make_builtin_print();
    make_builtin_read_int();
    make_builtin_read_string();

    init_int_tp();
    init_float_type();
    init_array_tp();
    init_bool_tp();
    init_str_tp();
    init_tp_tp();
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
void Interpreter::add_function_definition(const std::string &name, FunctionASTNode *fn)
{
    function_definitions[name] = fn;
}

std::shared_ptr<Function> Interpreter::push_function(const std::string& name)
{
    auto func = std::make_shared<Function>();
    func->name = name;
    function_stack.push_back(func);
    scope_stack.push_back(func->function_scope);

    return func;
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
