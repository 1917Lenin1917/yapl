//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/IntegerValue.hpp"

#include <memory>

namespace yapl {

void ArrayValue::make_size()
{
	const Token name{ TOKEN_TYPE::IDENTIFIER, new char[]("size")  };
	std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
	const Token return_type{ TOKEN_TYPE::IDENTIFIER, new char[]("int") };
	auto body = std::make_unique<BuiltinCustomVisitFunctionASTNode>([&](std::shared_ptr<Function> f_obj)
	{
		return std::make_unique<IntegerValue>(this->value.size());
	});

	auto f = std::make_unique<FunctionASTNode>(
			std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list, -1), return_type)),
																									 std::move(body));
	m_methods.push_back(std::move(f));
	m_method_definitions["size"] = m_methods[m_methods.size() - 1].get();
}

void ArrayValue::make_append()
{
	const Token name{ TOKEN_TYPE::IDENTIFIER, new char[]("append")  };
	std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
	arg_list.push_back(std::make_unique<FunctionArgumentASTNode>(Token{TOKEN_TYPE::IDENTIFIER, new char[]("value")}, Token{TOKEN_TYPE::IDENTIFIER, new char[]("any")}));
	const Token return_type{ TOKEN_TYPE::IDENTIFIER, new char[]("void") };
	auto body = std::make_unique<BuiltinCustomVisitFunctionASTNode>([&](std::shared_ptr<Function> f_obj)
	{
		auto arg = f_obj->function_scope->vars[f_obj->argument_names[0]];
		this->value.push_back(arg->value->Copy());
		return nullptr;
		// return std::make_unique<IntegerValue>(this->value.size());
	});

	auto f = std::make_unique<FunctionASTNode>(
			std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list, 1), return_type)),
																									 std::move(body));
	m_methods.push_back(std::move(f));
	m_method_definitions["append"] = m_methods[m_methods.size() - 1].get();
}

void ArrayValue::make_pop()
{
	const Token name{ TOKEN_TYPE::IDENTIFIER, new char[]("pop")  };
	std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
	const Token return_type{ TOKEN_TYPE::IDENTIFIER, new char[]("void") };
	auto body = std::make_unique<BuiltinCustomVisitFunctionASTNode>([&](std::shared_ptr<Function> f_obj)
	{
        this->value.pop_back();
        return nullptr;
	});

	auto f = std::make_unique<FunctionASTNode>(
			std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list, -1), return_type)),
																									 std::move(body));
	m_methods.push_back(std::move(f));
	m_method_definitions["pop"] = m_methods[m_methods.size() - 1].get();
}
ArrayValue::ArrayValue(std::vector<std::shared_ptr<Value>>& value)
		:Value(VALUE_TYPE::ARRAY), value(std::move(value))
{
	make_size();
	make_append();
	make_pop();
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
	std::vector<std::shared_ptr<Value>> ret_value;
	for (const auto& v : value)
	{
		ret_value.push_back(v->Copy());
	}
	return std::make_unique<ArrayValue>(ret_value);
}

std::shared_ptr<Value> ArrayValue::OperatorIndex(const std::shared_ptr<Value> &idx)
{
	if (idx->type != VALUE_TYPE::INTEGER)
		throw std::runtime_error("Invalid indexing type");
	const auto& v = value.at(dynamic_cast<IntegerValue*>(idx.get())->value);
	return std::move(v->Copy());
}

void ArrayValue::OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val)
{
	if (idx->type != VALUE_TYPE::INTEGER)
		throw std::runtime_error("Invalid indexing type");
	auto& v = value[dynamic_cast<IntegerValue*>(idx.get())->value];
	v->Set(new_val);
}


std::shared_ptr<Value> ArrayValue::UnaryMinus()
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::UnaryPlus()
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::UnaryNot()
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinaryPlus(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinaryMinus(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinarySlash(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinaryTimes(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinaryMOD(const std::shared_ptr<Value> &other)
{
    return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinaryLT(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinaryGT(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinaryLQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinaryGQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}

std::shared_ptr<Value> ArrayValue::BinaryEQ(const std::shared_ptr<Value> &other)
{
	return nullptr;
}
}