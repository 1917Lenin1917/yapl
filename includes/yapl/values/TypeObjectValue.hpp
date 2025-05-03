//
// Created by lenin on 29.04.2025.
//

#pragma once

#include "Value.hpp"
#include "TypeObject.hpp"

namespace yapl {

class TypeObjectValue : public Value
{
public:
    TypeObject* value;

    explicit TypeObjectValue(TypeObject* value);

    [[nodiscard]] std::string print() const override;
    [[nodiscard]] std::unique_ptr<Value> Copy() const override;

};

inline TypeObject *TypeObjectTypeObject = nullptr;

void init_tp_methods(TypeObject *tp);

static void init_tp_tp()
{
    TypeObjectTypeObject = new TypeObject{"type"};

    init_base_methods(TypeObjectTypeObject);
    init_tp_methods(TypeObjectTypeObject);


}
}
