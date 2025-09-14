//
// Created by lenin on 14.09.2025.
//

#include <utility>

#include "yapl/values/BuiltinFunctionValue.hpp"

namespace yapl {

BuiltinFunctionValue::BuiltinFunctionValue(std::string name, std::function<void(ByteCodeVM& VM)> func)
  :Value(VALUE_TYPE::BUILTIN_FUNCTION, BuiltinFunctionTypeObject), name(std::move(name)), func(std::move(func))
{
}

void init_builtin_function_methods(TypeObject *tp)
{
}
}
