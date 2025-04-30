//
// Created by lenin on 02.12.2024.
//

#pragma once

#include <memory>

#include "Value.hpp"

namespace yapl {

class StringValue final : public Value
{
public:
	std::string value; // probably should be a const char* ??

	explicit StringValue(std::string value);

	[[nodiscard]] std::string print() const override;
	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

};

inline TypeObject* StringTypeObject = nullptr;

void init_str_methods(TypeObject* tp);

void init_str_tp();

}