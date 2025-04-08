//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/FloatValue.hpp"

namespace yapl {

FloatValue::FloatValue(const float value)
		:Value(VALUE_TYPE::FLOAT), value(value) {}

std::string FloatValue::print() const
{
	return std::to_string(value);
}

std::unique_ptr<Value> FloatValue::Copy() const
{
	return std::make_unique<FloatValue>(value);
}

std::shared_ptr<Value> FloatValue::UnaryMinus()
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::UnaryPlus()
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::UnaryNot()
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::BinaryPlus(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::BinaryMinus(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::BinarySlash(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::BinaryTimes(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::BinaryLT(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::BinaryGT(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::BinaryLQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::BinaryGQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> FloatValue::BinaryEQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}
}