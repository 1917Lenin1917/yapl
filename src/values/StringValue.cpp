//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/StringValue.hpp"

namespace yapl {

StringValue::StringValue(std::string value)
		:Value(VALUE_TYPE::STRING), value(std::move(value)) {}

std::string StringValue::print() const
{
	return value;
}

std::unique_ptr<Value> StringValue::Copy() const
{
	return std::make_unique<StringValue>(value);
}

std::shared_ptr<Value> StringValue::UnaryMinus()
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::UnaryPlus()
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::UnaryNot()
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::BinaryPlus(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::BinaryMinus(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::BinarySlash(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::BinaryTimes(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::BinaryMOD(const std::shared_ptr<Value> &other)
{
    return nullptr;
}

std::shared_ptr<Value> StringValue::BinaryLT(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::BinaryGT(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::BinaryLQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::BinaryGQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> StringValue::BinaryEQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}
}