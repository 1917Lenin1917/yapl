//
// Created by lenin on 15.11.2024.
//

#include <memory>
#include <sstream>
#include <filesystem>
#include <fstream>

#include "yapl/Interpreter.hpp"

#include <cmath>
#include <yapl/Parser.hpp>

#include "yapl/Lexer.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/TypeObjectValue.hpp"
#include <yapl/values/DictValue.hpp>
#include <yapl/values/FunctionValue.hpp>


namespace yapl {

std::vector<std::string> get_lines_from_text(const std::string& text)
{
    std::vector<std::string> res;
    std::stringstream ss { text };
    while (!ss.eof())
    {
        std::string line;
        std::getline(ss, line);
        res.push_back(line);
    }
    return res;
}

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

    MAKE_METHOD(this, "sqrt", "any", ARG("num", "any"))
    {
        auto arg = f_obj->function_scope->vars["num"];
        auto num = as_int(arg->value.get())->value;
        return mk_float(std::sqrt(num));
    };

    init_int_tp();
    init_float_type();
    init_array_tp();
    init_bool_tp();
    init_str_tp();
    init_tp_tp();
    init_dict_tp();
    init_function_tp();
    init_size_iterator_type();


    types[IntegerTypeObject->name] = mk_type(IntegerTypeObject);
    types[FloatTypeObject->name] = mk_type(FloatTypeObject);
    types[ArrayTypeObject->name] = mk_type(ArrayTypeObject);
    types[BooleanTypeObject->name] = mk_type(BooleanTypeObject);
    types[StringTypeObject->name] = mk_type(StringTypeObject);
    types[TypeObjectTypeObject->name] = mk_type(TypeObjectTypeObject);
    types[DictTypeObject->name] = mk_type(DictTypeObject);
    types[FunctionTypeObject->name] = mk_type(FunctionTypeObject);
    types[SizeIteratorTypeObject->name] = mk_type(SizeIteratorTypeObject);

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
    if (current_module)
        return current_module->function_definitions.contains(name);
    return function_definitions.contains(name);
}

std::shared_ptr<Value> Interpreter::AddFunctionDefinition(const std::string &name, FunctionASTNode *fn)
{
    if (current_module)
    {
        current_module->function_definitions[name] = fn;
        return mk_func(name, fn);
    }
    function_definitions[name] = fn;
    return mk_func(name, fn);
}

std::shared_ptr<Function> Interpreter::PushFunction(const std::string& name)
{
    if (current_module)
    {
        auto func = std::make_shared<Function>();
        func->name = name;
        current_module->function_stack.push_back(func);
        current_module->scope_stack.push_back(func->function_scope);

        return func;
    }
    auto func = std::make_shared<Function>();
    func->name = name;
    function_stack.push_back(func);
    scope_stack.push_back(func->function_scope);

    return func;
}

std::shared_ptr<Variable> Interpreter::AddVariable(const std::string &name, bool is_const, VPtr value)
{
    if (current_module)
    {
        auto scope = current_module->scope_stack[current_module->scope_stack.size() - 1];
        scope->vars[name] = std::make_shared<Variable>(is_const, value->type, value);
        return scope->vars[name];
    }
    auto scope = scope_stack[scope_stack.size() - 1];
    scope->vars[name] = std::make_shared<Variable>(is_const, value->type, value);
    return scope->vars[name];
}

/*
 * When calling this method, we assume that we're at the top of the call stack. So we can only see last stack's and its parent's vars as well as global vars
 */
std::shared_ptr<Variable> Interpreter::GetVariable(const std::string &name)
{
    // for module
    if (current_module)
    {
        // look through newest scope and its parents
        auto scope = current_module->scope_stack[current_module->scope_stack.size() - 1];
        while (scope)
        {
            if (scope->vars.contains(name))
            {
                return scope->vars.at(name);
            }
            scope = scope->parent_scope;
        }

        // look in globals
        if (const auto global_stack = current_module->scope_stack[0]; global_stack->vars.contains(name))
        {
            return global_stack->vars.at(name);
        }

        return nullptr;
    }

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

void Interpreter::Export(const std::string& name, const std::shared_ptr<Variable>& var)
{
    current_module->exported[name] = var;
}

std::shared_ptr<Function> Interpreter::GetCurrentFunction()
{
    if (current_module)
        return current_module->function_stack.empty() ? nullptr : current_module->function_stack[current_module->function_stack.size() - 1];
    return function_stack.empty() ? nullptr : function_stack[function_stack.size() - 1];
}

std::shared_ptr<Scope> Interpreter::GetCurrentScope()
{
    if (current_module)
    {
        return current_module->scope_stack[current_module->scope_stack.size() - 1];
    }
    return scope_stack[scope_stack.size() - 1];
}

void Interpreter::pop_function()
{
    if (current_module)
    {
        current_module->function_stack.pop_back();
        return;
    }
    function_stack.pop_back();
}
void Interpreter::PushScope()
{
    if (current_module)
    {
        current_module->scope_stack.push_back(std::make_shared<Scope>());
        return;
    }
    scope_stack.push_back(std::make_shared<Scope>());
}
void Interpreter::PopScope()
{
    if (current_module)
    {
        current_module->scope_stack.pop_back();
        return;
    }
    scope_stack.pop_back();
}

std::shared_ptr<Module> Interpreter::LoadModule(const std::string &name)
{
    if (auto mod = std::find_if(modules.begin(), modules.end(), [&name](const auto& module) { return module->name == name; }); mod != modules.end())
    {
        return *mod;
    }
    auto mod_path = base_path / name;

    std::ifstream t(mod_path.string());
    std::string text((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
    Lexer lexer {text};

    auto tokens = lexer.make_tokens();

    auto lines = get_lines_from_text(text);
    Parser parser{ tokens, name, lines };

    auto module_root = parser.parse_root();

    auto module = std::make_shared<Module>(std::move(module_root), name);
    modules.push_back(module);

    module->scope_stack.push_back(std::make_unique<Scope>()); // make module scope
    return module;
}

std::shared_ptr<Module> Interpreter::GetModule(const std::string &name)
{
    for (auto md : modules)
        if (md->name == name)
            return md;

    return nullptr;
}
}
