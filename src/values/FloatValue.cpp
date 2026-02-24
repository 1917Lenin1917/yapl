//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/FloatValue.hpp"

#include "yapl/Serialization.hpp"

namespace yapl {

FloatValue::FloatValue(float value)
		:Value(VALUE_TYPE::FLOAT, FloatTypeObject), value(value) {}

std::unique_ptr<Value> FloatValue::Copy() const
{
	return std::make_unique<FloatValue>(value);
}

std::vector<std::byte> FloatValue::Serialize()
{
    std::vector<std::byte> buffer;

    const auto moduleLength = module.size();

    buffer.reserve(2 + 4 + module.size());

    appendByte(buffer, static_cast<std::byte>(VALUE_TYPE::INTEGER));
    appendUint16(buffer, moduleLength);
    if (moduleLength)
      appendStringBytes(buffer, module);
    appendUint16(buffer, 32);
    appendFloat(buffer, value);

    return buffer;
}


void init_float_methods(TypeObject* tp)
{

}

}
