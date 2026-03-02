//
// Created by lenin on 02.12.2024.
//

#include <utility>

#include "yapl/values/Value.hpp"

#include "yapl/ByteCodeVM.hpp"
#include "yapl/values/StringValue.hpp"
#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/TypeObjectValue.hpp"

namespace yapl {
    std::string value_type_to_string(const VALUE_TYPE vt)
    {
        switch (vt) {
            case VALUE_TYPE::NONE: return "NONE";
            case VALUE_TYPE::INTEGER: return "int";
            case VALUE_TYPE::FLOAT: return "float";
            case VALUE_TYPE::BOOL: return "bool";
            case VALUE_TYPE::STRING: return "string";
            case VALUE_TYPE::ARRAY: return "array";
            case VALUE_TYPE::TYPE: return "type";
            case VALUE_TYPE::DICT: return "dict";
            case VALUE_TYPE::USER_DEFINED: return "custom";
            case VALUE_TYPE::FUNCTION: return "fn";
            case VALUE_TYPE::BUILTIN_FUNCTION: return "builtin_fn";
            case VALUE_TYPE::ITERATOR: return "iterator";
            case VALUE_TYPE::UNDEFINED: return "undefined";
            case VALUE_TYPE::CODE_OBJECT: return "code_object";
            case VALUE_TYPE::MODULE: return "module";
            default: return "unhandled";
        }
    }

Value::Value(const VALUE_TYPE type, TypeObject* tp)
	:tp(tp), type(type)
{
}

std::string Value::print()
{
  {
    if (const auto str = tp->nb_str)
    {
      auto str_v  = str(shared_from_this());
      return as_str(str_v.get())->value;
    }

    return std::format("<object of type '{}' at {}>", tp->name, static_cast<const void*>(this));
  }
}


std::shared_ptr<Value> Value::Call(const std::vector<VPtr> &args)
{
    return NotImplemented;
}

std::shared_ptr<ArrayValue> Value::GetMethods() const
{
    std::vector<std::shared_ptr<Value>> values;
    for(const auto& [name, ptr] : tp->methods)
    {
        values.push_back(std::make_unique<StringValue>(name));
    }
    return std::make_unique<ArrayValue>(values);
}

VPtr Value::dispatch(unop_fn slot, const char *opname)
{
    if (!slot) return NotImplemented;

    VPtr r = slot(shared_from_this());
    return r != NotImplemented ? r : NotImplemented;
}

VPtr Value::dispatch(yapl::binop_fn slot, const yapl::VPtr &rhs, const char *opname)
{
    if (!slot) return NotImplemented;

    VPtr r = slot(shared_from_this(), rhs);
    return r != NotImplemented ? r : NotImplemented;
}


void init_base_methods(TypeObject* tp)
{
    //VPtr(const VPtr&, const std::string& attr_name)
    tp->nb_getattr = [](const VPtr& self, const std::string& attr_name) -> VPtr
    {
        return self->GetField(attr_name);
    };
    // VPtr(const VPtr& self, const std::string& attr_name, const VPtr& value)
    tp->nb_setattr = [](const VPtr& self, const std::string& attr_name, const VPtr& value)
    {
        self->SetField(attr_name, value);
    };
}


std::tuple<std::shared_ptr<ArrayValue>, std::shared_ptr<DictValue>> get_args_kwargs(ByteCodeVM& VM)
{
    auto _kwargs = VM.m_Stack.top();
    auto kwargs = std::static_pointer_cast<DictValue>(_kwargs);
    VM.m_Stack.pop();

    auto _args = VM.m_Stack.top();
    auto args = std::static_pointer_cast<ArrayValue>(_args);
    VM.m_Stack.pop();

    return { args, kwargs };
}

}
