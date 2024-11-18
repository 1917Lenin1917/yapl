//
// Created by lenin on 17.11.2024.
//

#include "Value.hpp"

namespace yapl {
//
// IntegerValue
//

std::string IntegerValue::print() const
{
	return std::to_string(value);
}

std::unique_ptr<Value> IntegerValue::Copy() const
{
	return std::make_unique<IntegerValue>(value);
}


std::unique_ptr<Value> IntegerValue::UnaryMinus()
{
	return std::make_unique<IntegerValue>(-value);
}

std::unique_ptr<Value> IntegerValue::UnaryPlus()
{
	return std::make_unique<IntegerValue>(+value);
}

std::unique_ptr<Value> IntegerValue::UnaryNot()
{
	return std::make_unique<IntegerValue>(!value);
}

std::unique_ptr<Value> IntegerValue::BinaryPlus(const std::unique_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
		{
			return std::make_unique<IntegerValue>(value + dynamic_cast<IntegerValue*>(other.get())->value);
		}
		case VALUE_TYPE::BOOL:
		{
			return std::make_unique<BooleanValue>(value + dynamic_cast<IntegerValue*>(other.get())->value);
		}
		case VALUE_TYPE::FLOAT:
		{
			return std::make_unique<FloatValue>(static_cast<float>(value) + dynamic_cast<FloatValue*>(other.get())->value);
		}
		case VALUE_TYPE::STRING:
		{
			return std::make_unique<StringValue>(std::to_string(value) + dynamic_cast<StringValue*>(other.get())->value);
		}
		default:
		{
			std::cerr << std::format("Unhandled type() in BinaryPlus of objects  and .\n");
			return nullptr;
		}
	}
}

std::unique_ptr<Value> IntegerValue::BinaryMinus(const std::unique_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
		{
			return std::make_unique<IntegerValue>(value - dynamic_cast<IntegerValue*>(other.get())->value);
		}
		case VALUE_TYPE::BOOL:
		{
			return std::make_unique<BooleanValue>(value - dynamic_cast<IntegerValue*>(other.get())->value);
		}
		case VALUE_TYPE::FLOAT:
		{
			return std::make_unique<FloatValue>(static_cast<float>(value) - dynamic_cast<FloatValue*>(other.get())->value);
		}
		case VALUE_TYPE::STRING:
		{
			std::cerr << std::format("Cannot subtract string from number. This is not JS.\n");
			return nullptr;
		}
		default:
		{
			std::cerr << std::format("Unhandled type() in BinaryMinus of objects  and .\n");
			return nullptr;
		}
	}
}
std::unique_ptr<Value> IntegerValue::BinarySlash(const std::unique_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
		{
			return std::make_unique<IntegerValue>(value / dynamic_cast<IntegerValue*>(other.get())->value);
		}
		case VALUE_TYPE::BOOL:
		{
			return std::make_unique<BooleanValue>(value / dynamic_cast<IntegerValue*>(other.get())->value);
		}
		case VALUE_TYPE::FLOAT:
		{
			return std::make_unique<FloatValue>(static_cast<float>(value) / dynamic_cast<FloatValue*>(other.get())->value);
		}
		case VALUE_TYPE::STRING:
		{
			std::cerr << std::format("Cannot divide number by string. This is not JS. Maybe this operator should do something else later?\n");
			return nullptr;
		}
		default:
		{
			std::cerr << std::format("Unhandled type() in BinarySlash of objects  and .\n");
			return nullptr;
		}
	}
}

std::unique_ptr<Value> IntegerValue::BinaryTimes(const std::unique_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
		{
			return std::make_unique<IntegerValue>(value * dynamic_cast<IntegerValue*>(other.get())->value);
		}
		default: { return nullptr; }
	}
	return nullptr;
}

std::unique_ptr<Value> IntegerValue::BinaryLT(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> IntegerValue::BinaryGT(const std::unique_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
		{
			auto v = std::make_unique<BooleanValue>(value > dynamic_cast<IntegerValue*>(other.get())->value);
			return v;
		}
		default: { return nullptr; }
	}
	return nullptr;
}

std::unique_ptr<Value> IntegerValue::BinaryLQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> IntegerValue::BinaryGQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> IntegerValue::BinaryEQ(const std::unique_ptr<Value> &other)
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

std::string BooleanValue::print() const
{
	return value ? "true" : "false";
}

std::unique_ptr<Value> BooleanValue::Copy() const
{
	return std::make_unique<BooleanValue>(value);
}

std::unique_ptr<Value> BooleanValue::UnaryMinus()
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::UnaryPlus()
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::UnaryNot()
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::BinaryPlus(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::BinaryMinus(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::BinarySlash(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::BinaryTimes(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::BinaryLT(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::BinaryGT(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::BinaryLQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::BinaryGQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> BooleanValue::BinaryEQ(const std::unique_ptr<Value> &other)
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

std::string FloatValue::print() const
{
	return std::to_string(value);
}

std::unique_ptr<Value> FloatValue::Copy() const
{
	return std::make_unique<FloatValue>(value);
}

std::unique_ptr<Value> FloatValue::UnaryMinus()
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::UnaryPlus()
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::UnaryNot()
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::BinaryPlus(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::BinaryMinus(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::BinarySlash(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::BinaryTimes(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::BinaryLT(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::BinaryGT(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::BinaryLQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::BinaryGQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> FloatValue::BinaryEQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::string StringValue::print() const
{
	return value;
}

std::unique_ptr<Value> StringValue::Copy() const
{
	return std::make_unique<StringValue>(value);
}

std::unique_ptr<Value> StringValue::UnaryMinus()
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::UnaryPlus()
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::UnaryNot()
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::BinaryPlus(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::BinaryMinus(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::BinarySlash(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::BinaryTimes(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::BinaryLT(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::BinaryGT(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::BinaryLQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::BinaryGQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> StringValue::BinaryEQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}
}
