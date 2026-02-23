//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/ArrayValue.hpp"
#include "yapl/values/IntegerValue.hpp"
#include "yapl/values/BuiltinFunctionValue.hpp"

#include <memory>

#include "yapl/ByteCodeVM.hpp"

namespace yapl {

ArrayValue::ArrayValue(std::vector<std::shared_ptr<Value>>& value)
		:Value(VALUE_TYPE::ARRAY, ArrayTypeObject), value(std::move(value)) // FIXME: this could potentially cause a lot of issues, probably should just make a copy, or take &&
{
}

ArrayValue::ArrayValue(std::vector<std::shared_ptr<Value>>&& value)
        :Value(VALUE_TYPE::ARRAY, ArrayTypeObject), value(std::move(value)) // FIXME: this could potentially cause a lot of issues, probably should just make a copy, or take &&
{

}

std::unique_ptr<Value> ArrayValue::Copy() const
{
	std::vector<std::shared_ptr<Value>> ret_value;
	for (const auto& v : value)
	{
		ret_value.push_back(v->Copy());
	}
	return std::make_unique<ArrayValue>(ret_value);
}

std::shared_ptr<Value> ArrayValue::OperatorIndex(const std::shared_ptr<Value> &idx)
{
	if (idx->type != VALUE_TYPE::INTEGER)
		throw std::runtime_error("Invalid indexing type");
	const auto& v = value.at(dynamic_cast<IntegerValue*>(idx.get())->value);
	return std::move(v->Copy());
}

void ArrayValue::OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val)
{
	if (idx->type != VALUE_TYPE::INTEGER)
		throw std::runtime_error("Invalid indexing type");
	value[dynamic_cast<IntegerValue*>(idx.get())->value] = new_val;
}

void init_array_methods(TypeObject* tp)
{
	const auto size_lambda = [](ByteCodeVM& VM)
	{
		auto _args = VM.m_Stack.top();
		VM.m_Stack.pop();
		auto args = as_arr(_args.get());

		auto self = as_arr(args->value[0].get());
		VM.m_Stack.push(mk_int(self->value.size()));
	};
	tp->methods["size"] = mk_builtin("size", size_lambda);

	const auto append_lambda = [](ByteCodeVM& VM)
	{
		auto _args = VM.m_Stack.top();
		VM.m_Stack.pop();
		auto args = as_arr(_args.get());

		auto self = as_arr(args->value[0].get());
		self->value.push_back(args->value[1]);
	};
	tp->methods["append"] = mk_builtin("append", append_lambda);

	const auto get_lambda = [](ByteCodeVM& VM)
	{
		auto _args = VM.m_Stack.top();
		VM.m_Stack.pop();
		auto args = as_arr(_args.get());

		auto self = as_arr(args->value[0].get());
		auto idx = as_int(args->value[1].get());
		VM.m_Stack.push(self->value[idx->value]);
	};
	tp->methods["get"] = mk_builtin("get", get_lambda);

	const auto set_lambda = [](ByteCodeVM& VM)
	{
		auto _args = VM.m_Stack.top();
		VM.m_Stack.pop();
		auto args = as_arr(_args.get());

		auto self = as_arr(args->value[0].get());
		auto idx = as_int(args->value[1].get());
		auto val = args->value[2];
		self->value[idx->value] = val;
	};
	tp->methods["set"] = mk_builtin("set", set_lambda);
}

}
