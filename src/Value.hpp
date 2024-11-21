//
// Created by lenin on 17.11.2024.
//

#pragma once
#include <memory>
#include <utility>
#include <iostream>
#include <format>
#include <unordered_map>
#include <vector>

#include "ASTNode.hpp"

namespace yapl {

class FunctionASTNode;


enum class VALUE_TYPE
{
	NONE = 0,
	INTEGER,
	FLOAT,
	BOOL,
	STRING,
	ARRAY,
};

class Value
{
protected:
	std::vector<std::unique_ptr<FunctionASTNode>> m_methods;
	std::unordered_map<std::string, FunctionASTNode*> m_method_definitions;
public:
	VALUE_TYPE type;

	explicit Value(VALUE_TYPE type);

	virtual ~Value() = default;

	[[nodiscard]] virtual std::string print() const = 0;
	[[nodiscard]] virtual std::unique_ptr<Value> Copy() const = 0;
	virtual void Set(const std::unique_ptr<Value>& v) { throw std::runtime_error("Not implemented"); };

	virtual std::unique_ptr<Value> UnaryMinus() = 0;
	virtual std::unique_ptr<Value> UnaryPlus() = 0;
	virtual std::unique_ptr<Value> UnaryNot() = 0;


	virtual std::unique_ptr<Value> BinaryPlus(const std::unique_ptr<Value>& other) = 0;
	virtual std::unique_ptr<Value> BinaryTimes(const std::unique_ptr<Value>& other) = 0;
	virtual std::unique_ptr<Value> BinaryMinus(const std::unique_ptr<Value>& other) = 0;
	virtual std::unique_ptr<Value> BinarySlash(const std::unique_ptr<Value>& other) = 0;
	virtual std::unique_ptr<Value> BinaryLT(const std::unique_ptr<Value>& other) = 0;
	virtual std::unique_ptr<Value> BinaryGT(const std::unique_ptr<Value>& other) = 0;
	virtual std::unique_ptr<Value> BinaryLQ(const std::unique_ptr<Value>& other) = 0;
	virtual std::unique_ptr<Value> BinaryGQ(const std::unique_ptr<Value>& other) = 0;
	virtual std::unique_ptr<Value> BinaryEQ(const std::unique_ptr<Value>& other) = 0;

	[[nodiscard]] virtual std::unique_ptr<Value> OperatorIndex(const std::unique_ptr<Value>& idx)
	{
		throw std::runtime_error("Unsupported operator!\n");
	}
	virtual void OperatorIndexSet(const std::unique_ptr<Value> &idx, std::unique_ptr<Value> new_val)
	{
		throw std::runtime_error("Unsupported operator!\n");
	}

	[[nodiscard]] FunctionASTNode* get_method_definition(const std::string& name)
	{
		if (m_method_definitions.contains(name))
			return m_method_definitions[name];

		std::cerr << "Unknown method " << name << "\n";
		return nullptr;
	}

	// [[nodiscard]] virtual std::unique_ptr<Value> OperatorMethod(const std::string& method_name, const std::vector<std::unique_ptr<Value>>& arguments)
	// {
	// 	if (!m_method_definitions.contains(method_name))
	// 		throw std::runtime_error("Unknown method!\n");
	// }
	// virtual void OperatorMethodSet(const std::string& method_name, const std::vector<std::unique_ptr<Value>>& arguments)
	// {
	// 	throw std::runtime_error("Unsupported operator!\n");
	// }
};

class IntegerValue final : public Value
{
public:
	int value;

	explicit IntegerValue(int value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	std::unique_ptr<Value> UnaryMinus() override;
	std::unique_ptr<Value> UnaryPlus() override;
	std::unique_ptr<Value> UnaryNot() override;

	void Set(const std::unique_ptr<Value> &v) override
	{
		value = dynamic_cast<IntegerValue*>(v.get())->value;
	}

	std::unique_ptr<Value> BinaryPlus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryMinus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinarySlash(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryTimes(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryEQ(const std::unique_ptr<Value> &other) override;

};

class BooleanValue final : public Value
{
public:
	bool value;

	explicit BooleanValue(bool value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	std::unique_ptr<Value> UnaryMinus() override;
	std::unique_ptr<Value> UnaryPlus() override;
	std::unique_ptr<Value> UnaryNot() override;

	std::unique_ptr<Value> BinaryPlus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryMinus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinarySlash(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryTimes(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryEQ(const std::unique_ptr<Value> &other) override;
};


class FloatValue final : public Value
{
public:
	float value;

	explicit FloatValue(const float value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	std::unique_ptr<Value> UnaryMinus() override;
	std::unique_ptr<Value> UnaryPlus() override;
	std::unique_ptr<Value> UnaryNot() override;

	std::unique_ptr<Value> BinaryPlus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryMinus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinarySlash(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryTimes(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryEQ(const std::unique_ptr<Value> &other) override;
};

class StringValue final : public Value
{
public:
	std::string value; // probably should be a const char* ??

	explicit StringValue(std::string value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	std::unique_ptr<Value> UnaryMinus() override;
	std::unique_ptr<Value> UnaryPlus() override;
	std::unique_ptr<Value> UnaryNot() override;

	std::unique_ptr<Value> BinaryPlus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryMinus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinarySlash(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryTimes(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryEQ(const std::unique_ptr<Value> &other) override;

};


class ArrayValue final : public Value
{
public:
	std::vector<std::unique_ptr<Value>> value;

	explicit ArrayValue(std::vector<std::unique_ptr<Value>>& value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	std::unique_ptr<Value> UnaryMinus() override;
	std::unique_ptr<Value> UnaryPlus() override;
	std::unique_ptr<Value> UnaryNot() override;

	std::unique_ptr<Value> BinaryPlus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryMinus(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinarySlash(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryTimes(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGT(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryLQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryGQ(const std::unique_ptr<Value> &other) override;
	std::unique_ptr<Value> BinaryEQ(const std::unique_ptr<Value> &other) override;

	[[nodiscard]] std::unique_ptr<Value> OperatorIndex(const std::unique_ptr<Value> &idx) override;
	void OperatorIndexSet(const std::unique_ptr<Value> &idx, std::unique_ptr<Value> new_val) override;
};

}
