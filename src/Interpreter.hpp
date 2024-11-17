//
// Created by lenin on 15.11.2024.
//

#pragma once
#include <utility>
#include <vector>
#include <unordered_map>

#include "Function.hpp"
#include "ASTNode.hpp"

namespace yapl {
class FunctionASTNode;

class Interpreter {
public:
    std::unordered_map<std::string, FunctionASTNode*> function_definitions;
    std::vector<std::shared_ptr<Function>> function_stack;
    std::vector<std::shared_ptr<Scope>> scope_stack;

    bool function_exists(const std::string& name) const
    {
      return function_definitions.contains(name);
    }

    void add_function_definition(const std::string& name, FunctionASTNode* fn)
    {
        function_definitions[name] = fn;
    }

    std::shared_ptr<Function> push_function(const std::string& name)
    {
        auto func = std::make_shared<Function>();
        func->name = name;
        function_stack.push_back(func);
        scope_stack.push_back(func->function_scope);

        return func;
    }
    void pop_function()
    {
        function_stack.pop_back();
    }

    void pop_scope()
    {
        scope_stack.pop_back();
    }

    FunctionASTNode* get_function_def(const std::string& name)
    {
        return function_definitions[name];
    }
};


}



