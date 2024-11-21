//
// Created by lenin on 17.11.2024.
//

#include "Function.hpp"

namespace yapl {
void Function::set_argument(std::unique_ptr<Variable> v, const std::string &name)
{
  argument_names.push_back(name);
  function_scope->vars[name] = std::move(v);
}

}
