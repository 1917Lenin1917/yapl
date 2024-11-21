//
// Created by lenin on 15.11.2024.
//

#include "Interpreter.hpp"

namespace yapl {

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
void Interpreter::pop_scope()
{
    scope_stack.pop_back();
}
FunctionASTNode *Interpreter::get_function_def(const std::string &name)
{
    return function_definitions[name];
}


}