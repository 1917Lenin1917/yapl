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
    explicit StringValue(std::string value, int repeat);

	std::shared_ptr<Value> OperatorIndex(const std::shared_ptr<Value> &idx) override;

	void Set(const std::shared_ptr<Value> &v) override
	{
			value = dynamic_cast<StringValue*>(v.get())->value;
	}


	[[nodiscard]] std::unique_ptr<Value> Copy() const override;

};

inline TypeObject* StringTypeObject = nullptr;

void init_str_methods(TypeObject* tp);

void init_str_tp();

}