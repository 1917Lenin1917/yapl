//
// Created by lenin on 17.11.2024.
//

#include "Value.hpp"

namespace yapl {

//
// Value
//
Value::Value(const VALUE_TYPE type = VALUE_TYPE::NONE)
	:type(type) {}


//
// IntegerValue
//
IntegerValue::IntegerValue(const int value)
		:Value(VALUE_TYPE::INTEGER), value(value) {}

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


//
// BooleanValue
//
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

ArrayValue::ArrayValue(std::vector<std::unique_ptr<Value> > &value)
		:Value(VALUE_TYPE::ARRAY), value(std::move(value))
{
	const Token name{ TOKEN_TYPE::IDENTIFIER, new char[]("size")  };
	std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
	const Token return_type{ TOKEN_TYPE::IDENTIFIER, new char[]("int") };
	auto body = std::make_unique<BuiltinCustomVisitFunctionASTNode>([&]()
	{
		return std::make_unique<IntegerValue>(this->value.size());
	});

	auto f = std::make_unique<FunctionASTNode>(
			std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list, -1), return_type)),
																									 std::move(body));
	m_methods.push_back(std::move(f));
	m_method_definitions["size"] = m_methods[m_methods.size() - 1].get();
}


std::string ArrayValue::print() const
{
	std::string ret_value = "[";
	auto size = value.size();
	for (size_t i = 0; i < size; i++)
	{
		ret_value += value[i]->print();
		if (i != size-1)
			ret_value += ", ";
	}
	return ret_value + "]";
}

std::unique_ptr<Value> ArrayValue::Copy() const
{
	std::vector<std::unique_ptr<Value>> ret_value;
	for (const auto& v : value)
	{
		ret_value.push_back(v->Copy());
	}
	return std::make_unique<ArrayValue>(ret_value);
}

std::unique_ptr<Value> ArrayValue::OperatorIndex(const std::unique_ptr<Value> &idx)
{
	if (idx->type != VALUE_TYPE::INTEGER)
		throw std::runtime_error("Invalid indexing type");
	const auto& v = value.at(dynamic_cast<IntegerValue*>(idx.get())->value);
	return std::move(v->Copy());
}

void ArrayValue::OperatorIndexSet(const std::unique_ptr<Value> &idx, std::unique_ptr<Value> new_val)
{
	if (idx->type != VALUE_TYPE::INTEGER)
		throw std::runtime_error("Invalid indexing type");
	auto& v = value[dynamic_cast<IntegerValue*>(idx.get())->value];
	v->Set(new_val);
}


std::unique_ptr<Value> ArrayValue::UnaryMinus()
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::UnaryPlus()
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::UnaryNot()
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::BinaryPlus(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::BinaryMinus(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::BinarySlash(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::BinaryTimes(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::BinaryLT(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::BinaryGT(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::BinaryLQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::BinaryGQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}

std::unique_ptr<Value> ArrayValue::BinaryEQ(const std::unique_ptr<Value> &other)
{
	return nullptr;
}
}
