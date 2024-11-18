//
// Created by lenin on 17.11.2024.
//

#pragma once
#include <memory>
#include <utility>
#include <iostream>
#include <format>

namespace yapl {

enum class VALUE_TYPE
{
	NONE = 0,
	INTEGER,
	FLOAT,
	BOOL,
	STRING,
};

class Value
{
public:
	VALUE_TYPE type;

	explicit Value(const VALUE_TYPE type = VALUE_TYPE::NONE)
		:type(type) {}

	virtual ~Value() = default;

	[[nodiscard]] virtual std::string print() const = 0;
	[[nodiscard]] virtual std::unique_ptr<Value> Copy() const = 0;

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
};

class IntegerValue final : public Value
{
public:
	int value;

	explicit IntegerValue(const int value)
		:Value(VALUE_TYPE::INTEGER), value(value) {}

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

class BooleanValue final : public Value
{
public:
	bool value;

	explicit BooleanValue(const bool value)
		:Value(VALUE_TYPE::BOOL), value(value) {}

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

	explicit FloatValue(const float value)
		:Value(VALUE_TYPE::FLOAT), value(value) {}

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

	explicit StringValue(std::string value)
		:Value(VALUE_TYPE::STRING), value(std::move(value)) {}

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
}
