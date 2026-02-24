//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/BooleanValue.hpp"

#include "yapl/Serialization.hpp"

namespace yapl {

BooleanValue::BooleanValue(const bool value)
		:Value(VALUE_TYPE::BOOL, BooleanTypeObject), value(value) {}

std::unique_ptr<Value> BooleanValue::Copy() const
{
	return std::make_unique<BooleanValue>(value);
}

std::vector<std::byte> BooleanValue::Serialize()
{
    std::vector<std::byte> buffer;

    const auto moduleLength = module.size();

    buffer.reserve(2 + 4 + module.size());

    appendByte(buffer, static_cast<std::byte>(VALUE_TYPE::BOOL));
    appendByte(buffer, static_cast<std::byte>(value ? 1 : 0));
    appendUint16(buffer, moduleLength);
    if (moduleLength)
      appendStringBytes(buffer, module);

    return buffer;
}

void init_bool_methods(TypeObject* tp)
{

}

}
