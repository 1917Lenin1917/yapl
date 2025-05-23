//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/StringValue.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/exceptions/RuntimeError.hpp"

namespace yapl {

StringValue::StringValue(std::string value)
		:Value(VALUE_TYPE::STRING, StringTypeObject), value(std::move(value)) {}

StringValue::StringValue(std::string value, int repeat)
        :Value(VALUE_TYPE::STRING, StringTypeObject)
{
    for(int i = 0; i < repeat; i++)
    {
        this->value += value;
    }
}

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

    StringTypeObject->nb_make = [](const std::vector<VPtr>& args) -> VPtr
    {
        if (args.empty())
            return mk_str("");
        if (args.size() == 1)
        {
            const auto& v = args[0];
            if (v->tp == StringTypeObject)
                return mk_str(as_str(v.get())->value);
            if (v->tp == IntegerTypeObject)
                return mk_str(std::to_string(as_int(v.get())->value));
            if (v->tp == FloatTypeObject)
                return mk_str(std::to_string(as_float(v.get())->value));
            // TODO: add more
            return NotImplemented;
        }
        if (args.size() == 2)
        {
            const auto& v = args[0];
            const auto& times = args[1];
            if (times->tp != IntegerTypeObject)
                throw RuntimeError("Invalid type when calling str(...). Expected 'int', got '" + times->tp->name + "' instead.");
            if (v->tp == StringTypeObject)
                return mk_str_rp(as_str(v.get())->value, as_int(times.get())->value);
            if (v->tp == IntegerTypeObject)
                return mk_str_rp(std::to_string(as_int(v.get())->value), as_int(times.get())->value);
            if (v->tp == FloatTypeObject)
                return mk_str_rp(std::to_string(as_float(v.get())->value), as_int(times.get())->value);
            // TODO: add more
            return NotImplemented;
        }
        return NotImplemented;
    };

    StringTypeObject->nb_add = [](const VPtr& self, const VPtr& other) -> VPtr
    {
        if (other->tp == IntegerTypeObject)
            return mk_str(as_str(self.get())->value + std::to_string(as_int(other.get())->value));

        return NotImplemented;
    };
}

}