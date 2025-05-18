//
// Created by lenin on 18.05.2025.
//

#include "yapl/values/UserDefinedValue.hpp"

yapl::UserDefinedValue::UserDefinedValue(TypeObject* tp)
  :Value(VALUE_TYPE::USER_DEFINED, tp) {}

std::string yapl::UserDefinedValue::print() const
{
  return std::format("<object of type '{}' at {}>", tp->name, static_cast<const void*>(this));
}

std::unique_ptr<yapl::Value> yapl::UserDefinedValue::Copy() const
{
  return nullptr;
}
