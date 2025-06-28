//
// Created by lenin on 29.04.2025.
//

#pragma once

#include "Value.hpp"
#include "TypeObject.hpp"
#include "StringValue.hpp"

namespace yapl {

class TypeObjectValue : public Value
{
public:
    TypeObject* value;

    explicit TypeObjectValue(TypeObject* value);

    [[nodiscard]] std::unique_ptr<Value> Copy() const override;

};

inline TypeObject *TypeObjectTypeObject = nullptr;

void init_tp_methods(TypeObject *tp);

static void init_tp_tp()
{
    TypeObjectTypeObject = new TypeObject{"type"};

    TypeObjectTypeObject->nb_str = [](const VPtr& self)
    {
        auto v = as_type(self.get());
        return mk_str(std::format("<class '{}'>", v->value->name));
    };

    init_base_methods(TypeObjectTypeObject);
    init_tp_methods(TypeObjectTypeObject);


}
}
