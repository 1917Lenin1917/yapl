//
// Created by lenin on 02.12.2024.
//

#pragma once

#include <string>

#include "Value.hpp"
#include "TypeObject.hpp"

#include "FloatValue.hpp"

namespace yapl {

class StringValue;

class IntegerValue final : public Value
{
public:
	int value;

	explicit IntegerValue(int value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

	void Set(const std::shared_ptr<Value> &v) override
	{
		value = dynamic_cast<IntegerValue*>(v.get())->value;
	}
};

inline TypeObject* IntegerTypeObject = nullptr;

void init_int_methods(TypeObject* tp);

static void init_int_tp()
{
    IntegerTypeObject = new TypeObject{ "int" };

    init_base_methods(IntegerTypeObject);
    init_int_methods(IntegerTypeObject);


    IntegerTypeObject->nb_add = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_int(as_int(self.get())->value + as_int(other.get())->value);
        if (other->tp == FloatTypeObject)
            return mk_float(as_int(self.get())->value + as_float(other.get())->value);

        return NotImplemented;
    };
    IntegerTypeObject->nb_sub = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_int(as_int(self.get())->value - as_int(other.get())->value);
        if (other->tp == FloatTypeObject)
            return mk_float(as_int(self.get())->value - as_float(other.get())->value);

        return NotImplemented;
    };
    IntegerTypeObject->nb_mul = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_int(as_int(self.get())->value * as_int(other.get())->value);
        if (other->tp == FloatTypeObject)
            return mk_float(as_int(self.get())->value * as_float(other.get())->value);

        return NotImplemented;
    };
    IntegerTypeObject->nb_div = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_int(as_int(self.get())->value / as_int(other.get())->value);
        if (other->tp == FloatTypeObject)
            return mk_float(as_int(self.get())->value / as_float(other.get())->value);

        return NotImplemented;
    };
    IntegerTypeObject->nb_mod = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_int(as_int(self.get())->value % as_int(other.get())->value);

        return NotImplemented;
    };
}

}
