//
// Created by lenin on 17.11.2024.
//

#pragma once
#include <memory>

#include "Value.hpp"

namespace yapl {
enum class VALUE_TYPE;
class Value;

class Variable
{
public:
  bool is_const = false;
  VALUE_TYPE type;
  std::shared_ptr<Value> value;
};
}
