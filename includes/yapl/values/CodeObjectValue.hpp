//
// Created by Максим Литвиненко on 11.09.2025.
//

#pragma once

#include <memory>

#include "yapl/values/Value.hpp"
#include "yapl/CodeObject.hpp"

namespace yapl {

class CodeObjectValue : public Value
{
public:
  explicit CodeObjectValue(std::shared_ptr<CodeObject> code_object);

  std::unique_ptr<Value> Copy() const override { return nullptr; }
  bool IsTruthy() const override { return false; }
  std::vector<std::byte> Serialize() override;
  static VPtr Deserialize(const std::vector<std::byte> &bytes, std::size_t &offset);

public:
  std::shared_ptr<CodeObject> code_object;
};

inline TypeObject* CodeObjectTypeObject = nullptr;

static void init_code_object_type_object()
{
  CodeObjectTypeObject = new TypeObject{ .name = "CodeObject" };
}

}
