//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/ArrayValue.hpp"

namespace yapl {

BooleanValue::BooleanValue(const bool value)
		:Value(VALUE_TYPE::BOOL), value(value) {}

std::string BooleanValue::print() const
{
	return value ? "true" : "false";
}

std::unique_ptr<Value> BooleanValue::Copy() const
{
	return std::make_unique<BooleanValue>(value);
}

std::shared_ptr<Value> BooleanValue::UnaryMinus()
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::UnaryPlus()
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::UnaryNot()
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::BinaryPlus(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::BinaryMinus(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::BinarySlash(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::BinaryTimes(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::BinaryLT(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::BinaryGT(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::BinaryLQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::BinaryGQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> BooleanValue::BinaryEQ(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
		{
			return std::make_unique<BooleanValue>(value == dynamic_cast<IntegerValue*>(other.get())->value);
		}
		case VALUE_TYPE::BOOL:
		{
			return std::make_unique<BooleanValue>(value == dynamic_cast<BooleanValue*>(other.get())->value);
		}
		case VALUE_TYPE::FLOAT:
		{
			return std::make_unique<BooleanValue>(static_cast<float>(value) == dynamic_cast<FloatValue*>(other.get())->value);
		}
		case VALUE_TYPE::STRING:
		{
			return std::make_unique<BooleanValue>(std::to_string(value) == dynamic_cast<StringValue*>(other.get())->value);
		}
		default:
		{
			std::cerr << std::format("Unhandled type() in BinaryEQ of objects  and .\n");
			return nullptr;
		}
	}
}
}