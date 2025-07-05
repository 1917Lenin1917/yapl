//
// Created by lenin on 15.11.2024.
//
#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include "Function.hpp"
#include "ASTNode.hpp"
#include "Scope.hpp"

namespace yapl {
class Value;
class FunctionASTNode;
class Function;
class Scope;
class Variable;
class BaseASTNode;

class Module
{
public:
    std::unique_ptr<BaseASTNode> ast;
    std::string name;

    std::vector<std::shared_ptr<Module>> modules;

    std::unordered_map<std::string, std::shared_ptr<Variable>> exported;

    std::unordered_map<std::string, std::shared_ptr<Value>> types;
    std::unordered_map<std::string, FunctionASTNode*> function_definitions;
    std::vector<std::shared_ptr<Function>> function_stack;
    std::vector<std::shared_ptr<Scope>> scope_stack;
};

class Interpreter {
public:
    std::filesystem::path base_path;

    std::unordered_map<std::string, std::shared_ptr<Value>> types;
    std::unordered_map<std::string, FunctionASTNode*> function_definitions;
    std::vector<std::shared_ptr<Function>> function_stack;
    std::vector<std::shared_ptr<Scope>> scope_stack;
    std::vector<std::unique_ptr<FunctionASTNode>> builtin_functions;

    std::vector<std::shared_ptr<Module>> modules;

    std::shared_ptr<Module> current_module = nullptr;

    Interpreter();
    bool function_exists(const std::string& name) const;
    std::shared_ptr<Variable> get_variable(const std::string& name) const;
    std::shared_ptr<Value> AddFunctionDefinition(const std::string& name, FunctionASTNode* fn);
    std::shared_ptr<Function> PushFunction(const std::string& name);
    std::shared_ptr<Variable> AddVariable(const std::string& name, bool is_const, std::shared_ptr<Value> value);
    std::shared_ptr<Variable> GetVariable(const std::string& name);
    void Export(const std::string& name, const std::shared_ptr<Variable>& var);
    void AddFunction();
    std::shared_ptr<Function> GetCurrentFunction();
    std::shared_ptr<Scope> GetCurrentScope();
    void pop_function();
    void PopScope();
    void PushScope();
    std::shared_ptr<Module> LoadModule(const std::string& name);
    std::shared_ptr<Module> GetModule(const std::string& name);

    void AddMethod(const std::string& method_name, std::unique_ptr<FunctionASTNode>&& function)
    {
        builtin_functions.push_back(std::move(function));
        function_definitions[method_name] = builtin_functions.back().get();
    }
};
}



