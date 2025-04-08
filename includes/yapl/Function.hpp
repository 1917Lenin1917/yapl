//
// Created by lenin on 17.11.2024.
//

#pragma once
#include <memory>
#include <vector>

#include "Scope.hpp"
#include "values/Value.hpp"

namespace yapl {
class Scope;
class Variable;
class Value;

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
