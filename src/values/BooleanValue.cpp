//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/BooleanValue.hpp"

namespace yapl {

BooleanValue::BooleanValue(const bool value)
		:Value(VALUE_TYPE::BOOL, BooleanTypeObject), value(value) {}

std::unique_ptr<Value> BooleanValue::Copy() const
{
	return std::make_unique<BooleanValue>(value);
}

void init_bool_methods(TypeObject* tp)
{

}

}