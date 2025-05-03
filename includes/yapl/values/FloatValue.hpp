//
// Created by lenin on 02.12.2024.
//

#pragma once

#include "Value.hpp"

namespace yapl {

class FloatValue final : public Value
{
public:
	float value;

	explicit FloatValue(float value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

};


inline TypeObject* FloatTypeObject = nullptr;

void init_float_methods(TypeObject* tp);

static void init_float_type()
{
    FloatTypeObject = new TypeObject{"float"};
    init_base_methods(FloatTypeObject);
    init_float_methods(FloatTypeObject);

    FloatTypeObject->nb_neg = [](const VPtr& self) -> VPtr
    {
        return mk_float(-as_float(self.get())->value);
    };
}

}