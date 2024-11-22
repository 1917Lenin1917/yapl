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
#pragma once

namespace yapl {
class Scope;
class Function;
class Variable;
class FunctionASTNode;
class FunctionDeclASTNode;
class BuiltinPrintFunctionBodyASTNode;
class BuiltinReadIntFunctionBodyASTNode;
class BuiltinReadStringFunctionBodyASTNode;
class FunctionArgumentASTNode;
class FunctionArgumentListASTNode;

class Interpreter {
    void make_builtin_push()
    {
        const Token name{ TOKEN_TYPE::IDENTIFIER, new char[]("print")  };
        std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
        const Token return_type{ TOKEN_TYPE::IDENTIFIER, new char[]("void") };
        auto f = std::make_unique<FunctionASTNode>(
            std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list, -1), return_type)),
                                                         std::move(std::make_unique<BuiltinPrintFunctionBodyASTNode>()));
        builtin_functions.push_back(std::move(f));
        function_definitions["print"] = builtin_functions[builtin_functions.size() - 1].get();
    }
    void make_builtin_read_int()
    {
        const Token name{ TOKEN_TYPE::IDENTIFIER, new char[]("read_int")  };
        std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
        const Token return_type{ TOKEN_TYPE::IDENTIFIER, new char[]("int") };
        auto f = std::make_unique<FunctionASTNode>(
            std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list), return_type)),
                                                         std::move(std::make_unique<BuiltinReadIntFunctionBodyASTNode>()));
        builtin_functions.push_back(std::move(f));
        function_definitions["read_int"] = builtin_functions[builtin_functions.size() - 1].get();
    }
    void make_builtin_read_string()
    {
        const Token name{ TOKEN_TYPE::IDENTIFIER, new char[]("read_str")  };
        std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
        const Token return_type{ TOKEN_TYPE::IDENTIFIER, new char[]("str") };
        auto f = std::make_unique<FunctionASTNode>(
            std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list), return_type)),
                                                         std::move(std::make_unique<BuiltinReadStringFunctionBodyASTNode>()));
        builtin_functions.push_back(std::move(f));
        function_definitions["read_str"] = builtin_functions[builtin_functions.size() - 1].get();
    }

public:
    std::unordered_map<std::string, FunctionASTNode*> function_definitions;
    std::vector<std::shared_ptr<Function>> function_stack;
    std::vector<std::shared_ptr<Scope>> scope_stack;

    std::vector<std::unique_ptr<FunctionASTNode>> builtin_functions;
    Interpreter()
    {
        scope_stack.push_back(std::make_unique<Scope>()); // make global scope

        make_builtin_push();
        make_builtin_read_int();
        make_builtin_read_string();
    }
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



