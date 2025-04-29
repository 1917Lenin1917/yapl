//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/IntegerValue.hpp"

#include <memory>

namespace yapl {

ArrayValue::ArrayValue(std::vector<std::shared_ptr<Value>>& value)
		:Value(VALUE_TYPE::ARRAY, ArrayTypeObject), value(std::move(value)) // FIXME: this could potentially cause a lot of issues, probably should just make a copy, or take &&
{
}


std::string ArrayValue::print() const
{
	std::string ret_value = "[";
	auto size = value.size();
	for (size_t i = 0; i < size; i++)
	{
		ret_value += value[i]->print();
		if (i != size-1)
			ret_value += ", ";
	}
	return ret_value + "]";
}

std::unique_ptr<Value> ArrayValue::Copy() const
{
	std::vector<std::shared_ptr<Value>> ret_value;
	for (const auto& v : value)
	{
		ret_value.push_back(v->Copy());
	}
	return std::make_unique<ArrayValue>(ret_value);
}

std::shared_ptr<Value> ArrayValue::OperatorIndex(const std::shared_ptr<Value> &idx)
{
	if (idx->type != VALUE_TYPE::INTEGER)
		throw std::runtime_error("Invalid indexing type");
	const auto& v = value.at(dynamic_cast<IntegerValue*>(idx.get())->value);
	return std::move(v->Copy());
}

void ArrayValue::OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val)
{
	if (idx->type != VALUE_TYPE::INTEGER)
		throw std::runtime_error("Invalid indexing type");
	auto& v = value[dynamic_cast<IntegerValue*>(idx.get())->value];
	v->Set(new_val);
}

void init_array_methods(TypeObject* tp)
{

    MAKE_METHOD(tp, "size", "int", ARG("this", "this"))
    {
        auto self = static_cast<ArrayValue*>(f_obj->function_scope->vars["this"]->value.get());
        return std::make_unique<IntegerValue>(self->value.size());
    };
    MAKE_METHOD(tp, "append", "void", ARG("this", "this"), ARG("value", "any"))
    {
        auto self = static_cast<ArrayValue*>(f_obj->function_scope->vars["this"]->value.get());
        auto arg = f_obj->function_scope->vars[f_obj->argument_names[0]];
        self->value.push_back(arg->value->Copy());
        return nullptr;
    };
    MAKE_METHOD(tp, "pop", "any", ARG("this", "this"))
    {
        auto self = static_cast<ArrayValue*>(f_obj->function_scope->vars["this"]->value.get());
        self->value.pop_back();
        return nullptr;
    };
}

}