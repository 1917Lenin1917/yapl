//
// Created by lenin on 17.11.2024.
//

#pragma once

#include <unordered_map>
#include <string>

#include "Variable.hpp"

namespace yapl {
class Variable;

class Scope
{
public:
  std::shared_ptr<Scope> parent_scope = nullptr;
  std::unordered_map<std::string, std::shared_ptr<Variable>> vars;
};

}