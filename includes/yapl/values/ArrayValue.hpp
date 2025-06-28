//
// Created by lenin on 02.12.2024.
//

#pragma once

#include "Value.hpp"
#include "StringValue.hpp"

namespace yapl {

class ArrayValue final : public Value
{
public:
	std::vector<std::shared_ptr<Value>> value;

	explicit ArrayValue(std::vector<std::shared_ptr<Value>>& value);
    explicit ArrayValue(std::vector<std::shared_ptr<Value>>&& value);

	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	[[nodiscard]] std::shared_ptr<Value> OperatorIndex(const std::shared_ptr<Value> &idx) override;
	void OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val) override;
};

inline TypeObject* ArrayTypeObject = nullptr;

void init_array_methods(TypeObject* tp);

static void init_array_tp()
{
    ArrayTypeObject = new TypeObject{ "array" };

		ArrayTypeObject->nb_str = [](const VPtr& self)
		{
			auto value = as_arr(self.get())->value;
			std::string ret_value = "[";
			auto size = value.size();
			for (size_t i = 0; i < size; i++)
			{
				ret_value += value[i]->print();
				if (i != size-1)
					ret_value += ", ";
			}
			return mk_str(ret_value + "]");
		};

    init_base_methods(ArrayTypeObject);
    init_array_methods(ArrayTypeObject);
}

}