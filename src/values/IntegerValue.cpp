//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/BooleanValue.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/FloatValue.hpp"
#include "yapl/values/ArrayValue.hpp"
#include "yapl/exceptions/RuntimeError.hpp"

#include <memory>
#include <vector>

#include "yapl/Serialization.hpp"

namespace yapl {

IntegerValue::IntegerValue(const int value)
		:Value(VALUE_TYPE::INTEGER, IntegerTypeObject), value(value)
{
}

std::unique_ptr<Value> IntegerValue::Copy() const
{
	return std::make_unique<IntegerValue>(value);
}

std::vector<std::byte> IntegerValue::Serialize()
{
    std::vector<std::byte> buffer;

    const auto moduleLength = module.size();

    buffer.reserve(2 + 4 + module.size());

    appendByte(buffer, static_cast<std::byte>(VALUE_TYPE::INTEGER));
    appendUint16(buffer, moduleLength);
    if (moduleLength)
      appendStringBytes(buffer, module);
    appendUint16(buffer, 32);
    appendUint32(buffer, value);

    return buffer;
}


void init_int_methods(TypeObject* tp)
{
    // MAKE_METHOD(tp, "str", "str", ARG("this", "this"))
    // {
    //     auto self = static_cast<IntegerValue*>(f_obj->function_scope->vars["this"]->value.get());
    //     return std::make_shared<StringValue>(std::to_string(self->value));
    // };
    // MAKE_METHOD(tp, "__add__", "int", ARG("this", "this"))
    // {
    //     auto self = f_obj->function_scope->vars["this"];
    //     const auto other = f_obj->function_scope->vars[f_obj->argument_names[0]];
    //     return self->value->BinaryPlus(other->value);
    // };
}

}
