//
// Created by lenin on 02.12.2024.
//

#include <utility>

#include "yapl/values/Value.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/TypeObjectValue.hpp"

namespace yapl {
    std::string value_type_to_string(VALUE_TYPE vt)
    {
        switch (vt) {
            case VALUE_TYPE::INTEGER: return "int";
            case VALUE_TYPE::STRING: return "str";
            default: return "unhandled";
        }
    }

Value::Value(const VALUE_TYPE type, TypeObject* tp)
	:type(type), tp(tp)
{
}

std::shared_ptr<ArrayValue> Value::GetMethods() const
{
    std::vector<std::shared_ptr<Value>> values;
    for(const auto& [name, ptr] : tp->method_dict)
    {
        values.push_back(std::make_unique<StringValue>(name));
    }
    return std::make_unique<ArrayValue>(values);
}

VPtr Value::dispatch(yapl::unop_fn slot, const char *opname)
{
    if (!slot) return NotImplemented;

    VPtr r = slot(std::shared_ptr<Value>(this));
    return r != NotImplemented ? r : NotImplemented;
}

VPtr Value::dispatch(yapl::binop_fn slot, const yapl::VPtr &rhs, const char *opname)
{
    if (!slot) return NotImplemented;

    VPtr r = slot(std::shared_ptr<Value>(this), rhs);
    return r != NotImplemented ? r : NotImplemented;
}


void init_base_methods(TypeObject* tp)
{
    MAKE_METHOD(tp, "type", "str", ARG("this", "this"))
    {
        auto self = f_obj->function_scope->vars["this"];
        return std::make_unique<TypeObjectValue>(self->value->tp);
    };

    MAKE_METHOD(tp, "dir", "array", ARG("this", "this"))
    {
        auto self = f_obj->function_scope->vars["this"];
        return self->value->GetMethods();
    };
}
}
