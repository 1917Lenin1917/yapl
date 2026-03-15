//
// Created by lenin on 29.04.2025.
//

#include <memory>

#include "yapl/ByteCodeVM.hpp"

#include "yapl/values/TypeObjectValue.hpp"
#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/DictValue.hpp"

namespace yapl {

TypeObjectValue::TypeObjectValue(yapl::TypeObject *value)
    :Value(VALUE_TYPE::TYPE, TypeObjectTypeObject), value(value) {}

std::unique_ptr<Value> TypeObjectValue::Copy() const
{
    return std::make_unique<TypeObjectValue>(this->value);
}


void init_tp_methods(TypeObject *tp)
{
    tp->nb_call = [](ByteCodeVM& VM, const VPtr& self)->VPtr
    {
        auto _kwargs = VM.m_Stack.top();
        auto kwargs = static_cast<DictValue*>(_kwargs.get());
        VM.m_Stack.pop();

        auto _args = VM.m_Stack.top();
        auto args = static_cast<ArrayValue*>(_args.get());
        const auto self_t = as_type(self.get());
        VM.m_Stack.pop();

        const std::size_t keyword_arg_count = kwargs->value.size();
        std::unordered_map<std::string, VPtr> keyword_arguments;

        keyword_arguments.reserve(keyword_arg_count);
        for (const auto& [first, second] : kwargs->value)
        {
          const auto name = static_cast<StringValue*>(first.get())->value;
          keyword_arguments.emplace(name, second);
        }

        const auto created = self_t->value->nb_make(args->value, keyword_arguments);
        VM.m_Stack.push(created);

        return nullptr;
    };
}

}