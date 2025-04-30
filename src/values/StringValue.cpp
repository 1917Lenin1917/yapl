//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/StringValue.hpp"
#include "yapl/values/IntegerValue.hpp"

namespace yapl {

StringValue::StringValue(std::string value)
		:Value(VALUE_TYPE::STRING, StringTypeObject), value(std::move(value)) {}

std::string StringValue::print() const
{
	return value;
}

std::unique_ptr<Value> StringValue::Copy() const
{
	return std::make_unique<StringValue>(value);
}


void init_str_methods(TypeObject* tp)
{

}

void init_str_tp()
{
    StringTypeObject = new TypeObject{"str"};
    init_base_methods(StringTypeObject);
    init_str_methods(StringTypeObject);

    StringTypeObject->nb_add = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_str(as_str(self.get())->value + std::to_string(as_int(other.get())->value));

        return NotImplemented;
    };
}

}