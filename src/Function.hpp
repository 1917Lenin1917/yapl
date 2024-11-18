//
// Created by lenin on 17.11.2024.
//

#pragma once
#include <memory>
#include <vector>

#include "Scope.hpp"

namespace yapl {

class Function
{
public:
  std::string name;
  std::shared_ptr<Scope> function_scope;
  std::vector<std::string> argument_names;
  std::unique_ptr<Value> return_value = nullptr;

  Function()
    :function_scope(std::make_shared<Scope>())
  {
    // TODO: add reference to global scope
  }

  void set_argument(std::unique_ptr<Variable> v, const std::string& name)
  {
    argument_names.push_back(name);
    function_scope->vars[name] = std::move(v);
  }
};
}
