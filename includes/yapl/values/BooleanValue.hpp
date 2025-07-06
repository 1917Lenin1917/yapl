//
// Created by lenin on 02.12.2024.
//

#pragma once

#include "Value.hpp"
#include "StringValue.hpp"

namespace yapl {

class BooleanValue final : public Value
{
public:
	bool value;

	explicit BooleanValue(bool value);

	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	[[nodiscard]] bool IsTruthy() const override { return value; }
};

inline TypeObject* BooleanTypeObject = nullptr;

void init_bool_methods(TypeObject* tp);

static void init_bool_tp()
{
    BooleanTypeObject = new TypeObject{"boolean"};

		BooleanTypeObject->nb_str = [](const VPtr& self)
		{
			auto v = as_bool(self.get());
			auto str = v->value ? "true" : "false";
			return mk_str(str);
		};

    BooleanTypeObject->nb_eq = [](const VPtr& self, const VPtr& other) -> VPtr
		{
        if (other->tp == BooleanTypeObject)
            return mk_bool(as_bool(self.get())->value == as_bool(other.get())->value);
        return NotImplemented;
		};

    BooleanTypeObject->nb_and = [](const VPtr& self, const VPtr& other) -> VPtr
		{
        if (other->tp == BooleanTypeObject)
            return mk_bool(as_bool(self.get())->value && as_bool(other.get())->value);
        return NotImplemented;
		};

    BooleanTypeObject->nb_or = [](const VPtr& self, const VPtr& other) -> VPtr
		{
        if (other->tp == BooleanTypeObject)
            return mk_bool(as_bool(self.get())->value || as_bool(other.get())->value);
        return NotImplemented;
		};

    init_base_methods(BooleanTypeObject);
    init_bool_methods(BooleanTypeObject);
}

}