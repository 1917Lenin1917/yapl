//
// Created by lenin on 02.12.2024.
//

#pragma once

#include "Value.hpp"
#include "StringValue.hpp"

namespace yapl {

class FloatValue final : public Value
{
public:
	float value;

	explicit FloatValue(float value);

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

    FloatTypeObject->nb_str = [](const VPtr& self)
    {
      return mk_str(std::to_string(as_float(self.get())->value));
    };
}

}