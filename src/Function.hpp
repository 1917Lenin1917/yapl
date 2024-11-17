//
// Created by lenin on 17.11.2024.
//

#pragma once
#include <memory>

#include "Scope.hpp"

namespace yapl {

class Function
{
public:
  std::string name;
  std::shared_ptr<Scope> function_scope;
  std::unique_ptr<Value> return_value = nullptr;

  Function()
    :function_scope(std::make_shared<Scope>())
  {
    // TODO: add reference to global scope
  }
};
}
