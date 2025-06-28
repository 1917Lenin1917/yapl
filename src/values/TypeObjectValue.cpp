//
// Created by lenin on 29.04.2025.
//

#include <memory>

#include "yapl/values/TypeObjectValue.hpp"
#include "yapl/values/ArrayValue.hpp"

namespace yapl {

TypeObjectValue::TypeObjectValue(yapl::TypeObject *value)
    :Value(VALUE_TYPE::TYPE, TypeObjectTypeObject), value(value) {}

std::unique_ptr<Value> TypeObjectValue::Copy() const
{
    return std::make_unique<TypeObjectValue>(this->value);
}


void init_tp_methods(TypeObject *tp)
{
    MAKE_METHOD_WITH_VARGS(tp, "make", "value", ARG("this", "this"))
    {
        // TODO: rewrite methods using variadic args
        auto self = static_cast<TypeObjectValue*>(f_obj->function_scope->vars["this"]->value.get());
        if (f_obj->function_scope->vars.contains("args"))
        {
            auto value = f_obj->function_scope->vars["args"]->value;
            return self->value->nb_make(as_arr(value.get())->value);
        }
        return self->value->nb_make({});
    };
}

}