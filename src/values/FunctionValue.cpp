//
// Created by lenin on 18.05.2025.
//

#include <utility>
#include <yapl/values/FunctionValue.hpp>
#include <yapl/values/ArrayValue.hpp>

#include "yapl/ByteCodeVM.hpp"

namespace yapl {

FunctionValue::FunctionValue(std::string name, std::shared_ptr<CodeObject>& code_object)
    :Value(VALUE_TYPE::FUNCTION, FunctionTypeObject), code_object(code_object), name(std::move(name)) {}

std::unique_ptr<Value> FunctionValue::Copy() const
{
    return nullptr;
}



void init_function_methods(TypeObject* tp)
{
  tp->nb_call = [](ByteCodeVM& VM, const VPtr& self)->VPtr
  {
    auto This = as_func(self.get());
    auto co = This->code_object.get();
    VM.Run(*co);

    return nullptr;
  };
}

}
