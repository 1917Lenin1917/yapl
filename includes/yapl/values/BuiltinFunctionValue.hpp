//
// Created by lenin on 14.09.2025.
//

#pragma once
#include <functional>

#include "Value.hpp"
#include "StringValue.hpp"

namespace yapl {

class BuiltinFunctionValue final : public Value
{
public:
  std::string name;
  std::function<void(ByteCodeVM& VM)> func;

  explicit BuiltinFunctionValue(std::string name, std::function<void(ByteCodeVM& VM)> func);

  std::unique_ptr<Value> Copy() const override { return nullptr; }
};

inline TypeObject* BuiltinFunctionTypeObject = nullptr;

void init_builtin_function_methods(TypeObject* tp);

static void init_builtin_function_tp()
{
  BuiltinFunctionTypeObject = new TypeObject{"function"};

  BuiltinFunctionTypeObject->nb_str = [](const VPtr& self)
  {
    auto name = as_builtin(self.get())->name;
    return mk_str(name);
  };

  BuiltinFunctionTypeObject->nb_call = [](ByteCodeVM& VM, const VPtr& self)->VPtr
  {
    auto This = as_builtin(self.get());
    This->func(VM);

    return nullptr;
  };

  init_base_methods(BuiltinFunctionTypeObject);
  init_builtin_function_methods(BuiltinFunctionTypeObject);

}

}
