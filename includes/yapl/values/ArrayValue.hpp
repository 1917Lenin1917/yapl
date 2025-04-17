//
// Created by lenin on 02.12.2024.
//

#pragma once

#include "Value.hpp"

namespace yapl {

class ArrayValue final : public Value
{
private:
	void make_size();
	void make_append();
	void make_pop();
public:
	std::vector<std::shared_ptr<Value>> value;

	explicit ArrayValue(std::vector<std::shared_ptr<Value>>& value);

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

	[[nodiscard]] std::shared_ptr<Value> OperatorIndex(const std::shared_ptr<Value> &idx) override;
	void OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val) override;
};
}