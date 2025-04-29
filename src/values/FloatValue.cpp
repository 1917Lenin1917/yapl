//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/FloatValue.hpp"

namespace yapl {

FloatValue::FloatValue(float value)
		:Value(VALUE_TYPE::FLOAT, FloatTypeObject), value(value) {}

std::string FloatValue::print() const
{
	return std::to_string(value);
}

std::unique_ptr<Value> FloatValue::Copy() const
{
	return std::make_unique<FloatValue>(value);
}


void init_float_methods(TypeObject* tp)
{

}

}