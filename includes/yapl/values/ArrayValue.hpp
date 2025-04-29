//
// Created by lenin on 02.12.2024.
//

#pragma once

#include "Value.hpp"

namespace yapl {

class ArrayValue final : public Value
{
public:
	std::vector<std::shared_ptr<Value>> value;

	explicit ArrayValue(std::vector<std::shared_ptr<Value>>& value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	[[nodiscard]] std::shared_ptr<Value> OperatorIndex(const std::shared_ptr<Value> &idx) override;
	void OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val) override;
};

inline TypeObject* ArrayTypeObject = nullptr;

void init_array_methods(TypeObject* tp);

static void init_array_tp()
{
    ArrayTypeObject = new TypeObject{ "array" };

    init_base_methods(ArrayTypeObject);
    init_array_methods(ArrayTypeObject);
}

}