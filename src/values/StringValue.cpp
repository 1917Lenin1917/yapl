//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/StringValue.hpp"

namespace yapl {

StringValue::StringValue(std::string value)
		:Value(VALUE_TYPE::STRING, StringTypeObject), value(std::move(value)) {}

std::string StringValue::print() const
{
	return value;
}

std::unique_ptr<Value> StringValue::Copy() const
{
	return std::make_unique<StringValue>(value);
}


void init_str_methods(TypeObject* tp)
{

}

}