//
// Created by lenin on 02.12.2024.
//

#pragma once

#include "Value.hpp"

namespace yapl {

class BooleanValue final : public Value
{
public:
	bool value;

	explicit BooleanValue(bool value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;
};

inline TypeObject* BooleanTypeObject = nullptr;

void init_bool_methods(TypeObject* tp);

static void init_bool_tp()
{
    BooleanTypeObject = new TypeObject{"boolean"};

    init_base_methods(BooleanTypeObject);
    init_bool_methods(BooleanTypeObject);
}

}