//
// Created by Максим Литвиненко on 27.08.2025.
//

#pragma once
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "ByteCode.hpp"


namespace yapl {
class Value;
class Variable;

enum class ParamKind {
  PositionalOrKeyword,
  KeywordOnly
};

struct Parameter {
  std::string name;
  ParamKind kind;
  bool has_default;
  std::size_t default_const_index;
  std::size_t local_index;
};

struct CodeObject
{
  std::vector<OpCode> op_codes;

  std::string name;
  std::size_t arg_count;
  std::vector<std::shared_ptr<Value>> constants;

  std::vector<std::string> locals;
  std::vector<std::string> names;
  std::vector<Parameter> params;
};

void print_code_object(const CodeObject& code_object, std::ostream& output = std::cout);


}
