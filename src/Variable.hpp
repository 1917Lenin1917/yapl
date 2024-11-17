//
// Created by lenin on 17.11.2024.
//

#pragma once
#include <memory>

#include "Value.hpp"

namespace yapl {

class Variable
{
public:
  bool is_const = false;
  VALUE_TYPE type = VALUE_TYPE::NONE;
  std::unique_ptr<Value> value;
};
}
