//
// Created by Максим Литвиненко on 27.08.2025.
//

#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "ByteCode.hpp"


namespace yapl {
class Value;
class Variable;

struct CodeObject
{
  std::vector<OpCode> op_codes;

  std::string name;
  std::size_t arg_count;
  std::vector<std::shared_ptr<Value>> constants;

  std::vector<std::string> locals;
  std::vector<std::string> names;
};

void print_code_object(const CodeObject& code_object, std::ostream& output = std::cout);


}
