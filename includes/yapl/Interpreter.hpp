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
    void make_builtin_print();
    void make_builtin_read_int();
    void make_builtin_read_string();
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
    void add_function_definition(const std::string& name, FunctionASTNode* fn);
    std::shared_ptr<Function> push_function(const std::string& name);
    void pop_function();
    void pop_scope();
    void push_scope();
    FunctionASTNode* get_function_def(const std::string& name);
};
}



