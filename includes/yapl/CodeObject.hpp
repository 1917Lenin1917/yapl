//
// Created by Максим Литвиненко on 27.08.2025.
//

#pragma once
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
  std::vector<OpCode> OpCodes;

  std::vector<std::string> Names;
  std::vector<std::shared_ptr<Variable>> Locals;
  std::vector<std::shared_ptr<Value>> Constants;

  // maybe move this to visitor?
  std::unordered_map<std::string, std::size_t> LocalsMap;
  std::unordered_map<std::string, std::size_t> ConstantsMap;
  std::unordered_map<std::string, std::size_t> NamesMap;
};

}
