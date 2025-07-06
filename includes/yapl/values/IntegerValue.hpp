//
// Created by lenin on 02.12.2024.
//

#pragma once

#include <string>

#include "Value.hpp"
#include "TypeObject.hpp"

#include "FloatValue.hpp"
#include "BooleanValue.hpp"
#include "StringValue.hpp"

namespace yapl {

class StringValue;

class IntegerValue final : public Value
{
public:
	int value;

	explicit IntegerValue(int value);

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

    IntegerTypeObject->nb_str = [](const VPtr& self)
    {
        return mk_str(std::to_string(as_int(self.get())->value));
    };

    IntegerTypeObject->nb_hash = [](const VPtr& self) -> std::size_t
    {
        return std::hash<int>{}(as_int(self.get())->value);
    };

    IntegerTypeObject->nb_make = [](const std::vector<VPtr>& args) -> VPtr
    {
        auto v = args.size() == 1 ? as_int(args[0].get())->value : 0;
        return std::make_unique<IntegerValue>(v);
    };

    IntegerTypeObject->nb_neg = [](const VPtr& self) -> VPtr
    {
        return mk_int(-as_int(self.get())->value);
    };

    IntegerTypeObject->nb_add = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_int(as_int(self.get())->value + as_int(other.get())->value);
        if (other->tp == FloatTypeObject)
            return mk_float(as_int(self.get())->value + as_float(other.get())->value);
        if (other->tp == StringTypeObject)
            return mk_str(std::to_string(as_int(self.get())->value) + as_str(other.get())->value);

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

    IntegerTypeObject->nb_lt = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_bool(as_int(self.get())->value < as_int(other.get())->value);

       return NotImplemented;
    };

    IntegerTypeObject->nb_gt = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_bool(as_int(self.get())->value > as_int(other.get())->value);

       return NotImplemented;
    };

    IntegerTypeObject->nb_eq = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_bool(as_int(self.get())->value == as_int(other.get())->value);
        return NotImplemented;
    };

    IntegerTypeObject->nb_nq = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_bool(as_int(self.get())->value != as_int(other.get())->value);
        return NotImplemented;
    };

    IntegerTypeObject->nb_lt = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_bool(as_int(self.get())->value < as_int(other.get())->value);
        return NotImplemented;
    };

    IntegerTypeObject->nb_le = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_bool(as_int(self.get())->value <= as_int(other.get())->value);
        return NotImplemented;
    };
}

}
