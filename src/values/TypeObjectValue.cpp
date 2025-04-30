//
// Created by lenin on 29.04.2025.
//

#include <memory>

#include "yapl/values/TypeObjectValue.hpp"

namespace yapl {

TypeObjectValue::TypeObjectValue(yapl::TypeObject *value)
    :Value(VALUE_TYPE::TYPE, TypeObjectTypeObject), value(value) {}

std::string TypeObjectValue::print() const
{
    return std::format("<class '{}'>", value->name);
}

std::unique_ptr<Value> TypeObjectValue::Copy() const
{
    return std::make_unique<TypeObjectValue>(this->value);
}


void init_tp_methods(TypeObject *tp)
{
    MAKE_METHOD(tp, "make", "value", ARG("this", "this"), ARG("value", "any"))
    {
        // TODO: rewrite methods using variadic args
        auto self = static_cast<TypeObjectValue*>(f_obj->function_scope->vars["this"]->value.get());
        if (f_obj->function_scope->vars.contains("value"))
        {
            auto value = f_obj->function_scope->vars["value"]->value;
            return self->value->nb_make({ value });
        }
        return self->value->nb_make({});
    };
}

}