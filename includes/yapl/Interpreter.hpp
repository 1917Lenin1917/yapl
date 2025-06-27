//
// Created by lenin on 15.11.2024.
//
#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <memory>

#include "Function.hpp"
#include "ASTNode.hpp"
#include "Scope.hpp"

namespace yapl {
class Value;
class FunctionASTNode;
class Function;
class Scope;
class Variable;

class Interpreter {
public:
    std::string module_name;

    std::unordered_map<std::string, std::shared_ptr<Value>> types;
    std::unordered_map<std::string, FunctionASTNode*> function_definitions;
    std::vector<std::shared_ptr<Function>> function_stack;
    std::vector<std::shared_ptr<Scope>> scope_stack;
    std::vector<std::unique_ptr<FunctionASTNode>> builtin_functions;

    Interpreter();
    bool function_exists(const std::string& name) const;
    std::shared_ptr<Variable> get_variable(const std::string& name) const;
    std::shared_ptr<Value> add_function_definition(const std::string& name, FunctionASTNode* fn);
    std::shared_ptr<Function> push_function(const std::string& name);
    std::shared_ptr<Variable> AddVariable(const std::string& name, bool is_const, std::shared_ptr<Value> value);
    std::shared_ptr<Variable> GetVariable(const std::string& name);
    std::shared_ptr<Function> GetCurrentFunction();
    void pop_function();
    void pop_scope();
    void push_scope();
    FunctionASTNode* get_function_def(const std::string& name);

    void AddMethod(const std::string& method_name, std::unique_ptr<FunctionASTNode>&& function)
    {
        builtin_functions.push_back(std::move(function));
        function_definitions[method_name] = builtin_functions.back().get();
    }
};
}



