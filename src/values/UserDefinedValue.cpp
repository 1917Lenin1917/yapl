//
// Created by lenin on 18.05.2025.
//

#include "yapl/values/UserDefinedValue.hpp"

#include <yapl/values/StringValue.hpp>

yapl::UserDefinedValue::UserDefinedValue(TypeObject* tp)
  :Value(VALUE_TYPE::USER_DEFINED, tp) {}

std::unique_ptr<yapl::Value> yapl::UserDefinedValue::Copy() const
{
  return nullptr;
}
