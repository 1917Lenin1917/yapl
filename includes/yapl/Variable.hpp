//
// Created by lenin on 17.11.2024.
//

#pragma once
#include <memory>

#include "values/Value.hpp"

namespace yapl {
enum class VALUE_TYPE : std::uint8_t;
class Value;

class Variable
{
public:
  bool is_const = false;
  VALUE_TYPE type;
  std::shared_ptr<Value> value;
  std::string module_name;
  std::string name;

  bool is_tdz = true;
};
}
