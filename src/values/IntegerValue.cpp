//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/ArrayValue.hpp"

#include <memory>

namespace yapl {

void IntegerValue::make_to_string()
{
    const Token name{ TOKEN_TYPE::IDENTIFIER, new char[]("to_string")  };
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
    const Token return_type{ TOKEN_TYPE::IDENTIFIER, new char[]("str") };
    auto body = std::make_unique<BuiltinCustomVisitFunctionASTNode>([&](std::shared_ptr<Function> f_obj)
    {
        return std::make_unique<StringValue>(std::to_string(this->value));
    });

    auto f = std::make_unique<FunctionASTNode>(
            std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list, -1), return_type)),
            std::move(body));
    m_methods.push_back(std::move(f));
    m_method_definitions["to_string"] = m_methods[m_methods.size() - 1].get();
}

IntegerValue::IntegerValue(const int value)
		:Value(VALUE_TYPE::INTEGER), value(value)
{
    make_to_string();
}

std::string IntegerValue::print() const
{
	return std::to_string(value);
}

std::unique_ptr<Value> IntegerValue::Copy() const
{
	return std::make_unique<IntegerValue>(value);
}


std::shared_ptr<Value> IntegerValue::UnaryMinus()
{
	return std::make_unique<IntegerValue>(-value);
}

std::shared_ptr<Value> IntegerValue::UnaryPlus()
{
	return std::make_unique<IntegerValue>(+value);
}

std::shared_ptr<Value> IntegerValue::UnaryNot()
{
	return std::make_unique<IntegerValue>(!value);
}

std::shared_ptr<Value> IntegerValue::BinaryPlus(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
			return std::make_unique<IntegerValue>(value + dynamic_cast<IntegerValue*>(other.get())->value);
		case VALUE_TYPE::BOOL:
			return std::make_unique<IntegerValue>(value + dynamic_cast<BooleanValue*>(other.get())->value);
		case VALUE_TYPE::FLOAT:
			return std::make_unique<FloatValue>(static_cast<float>(value) + dynamic_cast<FloatValue*>(other.get())->value);
		case VALUE_TYPE::STRING:
			return std::make_unique<StringValue>(std::to_string(value) + dynamic_cast<StringValue*>(other.get())->value);
		default:
		{
			std::cerr << std::format("Unhandled type() in BinaryPlus of objects  and .\n");
			return nullptr;
		}
	}
}

std::shared_ptr<Value> IntegerValue::BinaryMinus(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
			return std::make_unique<IntegerValue>(value - dynamic_cast<IntegerValue*>(other.get())->value);
		case VALUE_TYPE::BOOL:
			return std::make_unique<IntegerValue>(value - dynamic_cast<BooleanValue*>(other.get())->value);
		case VALUE_TYPE::FLOAT:
			return std::make_unique<FloatValue>(static_cast<float>(value) - dynamic_cast<FloatValue*>(other.get())->value);
		case VALUE_TYPE::STRING:
			std::cerr << std::format("Cannot subtract string from number. This is not JS.\n");
			return nullptr;
		default:
		{
			std::cerr << std::format("Unhandled type() in BinaryMinus of objects  and .\n");
			return nullptr;
		}
	}
}
std::shared_ptr<Value> IntegerValue::BinarySlash(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
			return std::make_unique<IntegerValue>(value / dynamic_cast<IntegerValue*>(other.get())->value);
		case VALUE_TYPE::BOOL:
			return std::make_unique<IntegerValue>(value / dynamic_cast<BooleanValue*>(other.get())->value);
		case VALUE_TYPE::FLOAT:
			return std::make_unique<FloatValue>(static_cast<float>(value) / dynamic_cast<FloatValue*>(other.get())->value);
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

std::shared_ptr<Value> IntegerValue::BinaryTimes(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
			return std::make_unique<IntegerValue>(value * dynamic_cast<IntegerValue*>(other.get())->value);
		case VALUE_TYPE::BOOL:
			return std::make_unique<IntegerValue>(value * dynamic_cast<BooleanValue*>(other.get())->value);
		case VALUE_TYPE::FLOAT:
			return std::make_unique<FloatValue>(static_cast<float>(value) * dynamic_cast<FloatValue*>(other.get())->value);
		case VALUE_TYPE::STRING:
		{
			const std::string &original = dynamic_cast<StringValue*>(other.get())->value;
			std::string res;
			for (size_t i = 0; i < value; i++)
				res += original;

			return std::make_unique<StringValue>(res);
		}
		case VALUE_TYPE::ARRAY:
		{
			std::vector<std::shared_ptr<Value>> values;
			auto arr = dynamic_cast<ArrayValue*>(other.get());
			size_t original_size = arr->value.size();
			size_t new_size = arr->value.size() * value;
			for (size_t i = 0; i < new_size; i++)
			{
				values.push_back(std::move(arr->value[i % original_size]->Copy()));
			}
			return std::make_unique<ArrayValue>(values);
		}
		default: { return nullptr; }
	}
}

std::shared_ptr<Value> IntegerValue::BinaryMOD(const std::shared_ptr<Value> &other)
{
    switch (other->type) {
        case VALUE_TYPE::INTEGER:
        {
            return std::make_unique<IntegerValue>(this->value % dynamic_cast<IntegerValue*>(other.get())->value);
        }
        default: { return nullptr; }
    }
}

std::shared_ptr<Value> IntegerValue::BinaryLT(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
			return std::make_unique<BooleanValue>(value < dynamic_cast<IntegerValue*>(other.get())->value);
		case VALUE_TYPE::BOOL:
			return std::make_unique<BooleanValue>(value < dynamic_cast<BooleanValue*>(other.get())->value);
		case VALUE_TYPE::FLOAT:
			return std::make_unique<BooleanValue>(static_cast<float>(value) < dynamic_cast<FloatValue*>(other.get())->value);
		default:
		{
			std::cerr << std::format("Unhandled type() in BinaryEQ of objects  and .\n");
			return nullptr;
		}
	}
}

std::shared_ptr<Value> IntegerValue::BinaryGT(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
			return std::make_unique<BooleanValue>(value > dynamic_cast<IntegerValue*>(other.get())->value);
		case VALUE_TYPE::BOOL:
			return std::make_unique<BooleanValue>(value > dynamic_cast<BooleanValue*>(other.get())->value);
		case VALUE_TYPE::FLOAT:
			return std::make_unique<BooleanValue>(static_cast<float>(value) > dynamic_cast<FloatValue*>(other.get())->value);
		default: { return nullptr; }
	}
}

std::shared_ptr<Value> IntegerValue::BinaryLQ(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
			return std::make_unique<BooleanValue>(value <= dynamic_cast<IntegerValue*>(other.get())->value);
		case VALUE_TYPE::BOOL:
			return std::make_unique<BooleanValue>(value <= dynamic_cast<BooleanValue*>(other.get())->value);
		case VALUE_TYPE::FLOAT:
			return std::make_unique<BooleanValue>(static_cast<float>(value) <= dynamic_cast<FloatValue*>(other.get())->value);
		default: { return nullptr; }
	}
}

std::shared_ptr<Value> IntegerValue::BinaryGQ(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
			return std::make_unique<BooleanValue>(value >= dynamic_cast<IntegerValue*>(other.get())->value);
		case VALUE_TYPE::BOOL:
			return std::make_unique<BooleanValue>(value >= dynamic_cast<BooleanValue*>(other.get())->value);
		case VALUE_TYPE::FLOAT:
			return std::make_unique<BooleanValue>(static_cast<float>(value) >= dynamic_cast<FloatValue*>(other.get())->value);
		default: { return nullptr; }
	}
}

std::shared_ptr<Value> IntegerValue::BinaryEQ(const std::shared_ptr<Value> &other)
{
	switch (other->type)
	{
		case VALUE_TYPE::INTEGER:
			return std::make_unique<BooleanValue>(value == dynamic_cast<IntegerValue*>(other.get())->value);
		case VALUE_TYPE::BOOL:
			return std::make_unique<BooleanValue>(value == dynamic_cast<BooleanValue*>(other.get())->value);
		case VALUE_TYPE::FLOAT:
			return std::make_unique<BooleanValue>(static_cast<float>(value) == dynamic_cast<FloatValue*>(other.get())->value);
		default:
		{
			std::cerr << std::format("Unhandled type() in BinaryEQ of objects  and .\n");
			return nullptr;
		}
	}
}
}