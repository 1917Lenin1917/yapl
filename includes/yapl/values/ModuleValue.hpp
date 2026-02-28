//
// Created by lenin on 26.02.2026.
//

#pragma once

#include <memory>

#include "Value.hpp"

namespace yapl {
struct Frame;

class ModuleValue : public Value
{
public:
  explicit ModuleValue(std::shared_ptr<Frame> module);

  std::unique_ptr<Value> Copy() const override { return nullptr; }
  bool IsTruthy() const override { return false; }

public:
  std::shared_ptr<Frame> module;
};

inline TypeObject* ModuleTypeObject = nullptr;

static void init_module_type_object()
{
  ModuleTypeObject = new TypeObject{ .name = "Module" };
}

}
