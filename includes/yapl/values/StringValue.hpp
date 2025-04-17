//
// Created by lenin on 02.12.2024.
//

#pragma once

#include "Value.hpp"
#include <memory>

namespace yapl {

class StringValue final : public Value
{
public:
	std::string value; // probably should be a const char* ??

	explicit StringValue(std::string value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	std::shared_ptr<Value> UnaryMinus() override;
	std::shared_ptr<Value> UnaryPlus() override;
	std::shared_ptr<Value> UnaryNot() override;

	std::shared_ptr<Value> BinaryPlus(const std::shared_ptr<Value> &other) override;
	std::shared_ptr<Value> BinaryMinus(const std::shared_ptr<Value> &other) override;
	std::shared_ptr<Value> BinarySlash(const std::shared_ptr<Value> &other) override;
	std::shared_ptr<Value> BinaryTimes(const std::shared_ptr<Value> &other) override;
    std::shared_ptr<Value> BinaryMOD(const std::shared_ptr<Value> &other) override;
	std::shared_ptr<Value> BinaryLT(const std::shared_ptr<Value> &other) override;
	std::shared_ptr<Value> BinaryGT(const std::shared_ptr<Value> &other) override;
	std::shared_ptr<Value> BinaryLQ(const std::shared_ptr<Value> &other) override;
	std::shared_ptr<Value> BinaryGQ(const std::shared_ptr<Value> &other) override;
	std::shared_ptr<Value> BinaryEQ(const std::shared_ptr<Value> &other) override;

};
}