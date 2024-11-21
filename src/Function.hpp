//
// Created by lenin on 17.11.2024.
//

#pragma once
#include <memory>
#include <vector>

#include "Scope.hpp"

namespace yapl {
class Scope;
class Value;
class Variable;

class Function
{
public:
  std::string name;
  std::shared_ptr<Scope> function_scope;
  std::vector<std::string> argument_names;
  std::shared_ptr<Value> return_value = nullptr;

  Function()
    :function_scope(std::make_shared<Scope>())
  {
    // TODO: add reference to global scope
  }

  void set_argument(std::unique_ptr<Variable> v, const std::string& name);
};
}
