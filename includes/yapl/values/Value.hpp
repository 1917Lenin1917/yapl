//
// Created by lenin on 02.12.2024.
//

#pragma once

#include <utility>
#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#include "yapl/ASTNode.hpp"
#include "TypeObject.hpp"

namespace yapl {
class DictValue;


#define mk_int(v) std::make_shared<IntegerValue>(v)
#define as_int(v) static_cast<IntegerValue*>(v)

#define mk_float(v) std::make_shared<FloatValue>(v)
#define as_float(v) static_cast<FloatValue*>(v)

#define mk_str(v) std::make_shared<StringValue>(v)
#define mk_str_rp(v, r) std::make_shared<StringValue>(v, r)
#define as_str(v) static_cast<StringValue*>(v)

#define mk_bool(v) std::make_shared<BooleanValue>(v)
#define as_bool(v) static_cast<BooleanValue*>(v)

#define mk_arr(v) std::make_shared<ArrayValue>(v)
#define as_arr(v) static_cast<ArrayValue*>(v)

#define mk_type(v) std::make_shared<TypeObjectValue>(v)
#define as_type(v) static_cast<TypeObjectValue*>(v)

#define mk_dict() std::make_shared<DictValue>()
#define as_dict(v) static_cast<DictValue*>(v)

#define mk_func(name, fn, globals) std::make_shared<FunctionValue>(name, fn, globals)
#define as_func(v) static_cast<FunctionValue*>(v)

#define mk_builtin(name, fn) std::make_shared<BuiltinFunctionValue>(name, fn)
#define as_builtin(v) static_cast<BuiltinFunctionValue*>(v)

#define mk_size_iter(iterable, len) std::make_shared<SizeIteratorValue>(iterable, len)
#define as_size_iter(v) static_cast<SizeIteratorValue*>(v)

#define mk_undefined() std::make_shared<UndefinedValue>()
#define as_undefined(v) static_cast<UndefinedValue*>(v)

#define mk_code_obj(co) std::make_shared<CodeObjectValue>(co)
#define as_code_obj(v) static_cast<CodeObjectValue*>(v)

#define mk_module(co) std::make_shared<ModuleValue>(co)
#define as_module(v) static_cast<ModuleValue*>(v)

struct TypeObject;

class FunctionASTNode;
class FunctionArgumentASTNode;
class FunctionArgumentListASTNode;
class BuiltinCustomVisitFunctionASTNode;
class FunctionDeclASTNode;

class ArrayValue;
class StringValue;

enum class VALUE_TYPE : std::uint8_t
{
	NONE = 0,
	INTEGER,
	FLOAT,
	BOOL,
	STRING,
	ARRAY,
  TYPE,
  DICT,
  USER_DEFINED,
  FUNCTION,
  BUILTIN_FUNCTION,
  ITERATOR,
  UNDEFINED,
  CODE_OBJECT,
	MODULE,
};

std::string value_type_to_string(VALUE_TYPE vt);

#define DEFINE_UNOP(method, slot_member, sym)        \
    VPtr method()                                    \
    {                                                \
        return dispatch(tp->slot_member, sym);       \
    }


#define DEFINE_BINOP(method, slot_member, sym)      \
    VPtr method(const VPtr& rhs)                    \
    {                                               \
        return dispatch(tp->slot_member, rhs, sym); \
    }

class Value : public std::enable_shared_from_this<Value>
{
public:
  std::unordered_map<std::string, VPtr> fields;
  std::string module;

  TypeObject* tp = nullptr;
	VALUE_TYPE type;

	explicit Value(VALUE_TYPE type, TypeObject* tp);
	virtual ~Value() = default;

	[[nodiscard]] virtual std::string print();
	[[nodiscard]] virtual std::unique_ptr<Value> Copy() const = 0;
	virtual void Set(const std::shared_ptr<Value>& v) { throw std::runtime_error("Not implemented"); }

  std::shared_ptr<Value> Call(const std::vector<VPtr>& args);

  [[nodiscard]] virtual bool IsTruthy() const { return false; };

  [[nodiscard]] std::shared_ptr<ArrayValue> GetMethods() const;

  DEFINE_UNOP(UnaryPlus, nb_pos, "+")
  DEFINE_UNOP(UnaryMinus, nb_neg, "-")
  DEFINE_UNOP(UnaryNot, nb_not, "!")

  DEFINE_BINOP(BinaryPlus,  nb_add, "+")
  DEFINE_BINOP(BinaryMinus, nb_sub, "-")
  DEFINE_BINOP(BinaryTimes, nb_mul, "*")
  DEFINE_BINOP(BinarySlash, nb_div, "/")
  DEFINE_BINOP(BinaryMOD,   nb_mod, "%")

  DEFINE_BINOP(BinaryLT, nb_lt,  "<")
  DEFINE_BINOP(BinaryGT, nb_gt,  ">")
  DEFINE_BINOP(BinaryLQ, nb_le,  "<=")
  DEFINE_BINOP(BinaryGQ, nb_ge,  ">=")
  DEFINE_BINOP(BinaryEQ, nb_eq,  "==")

  DEFINE_UNOP(Iter, nb_iter, "__iter__");
  DEFINE_UNOP(Next, nb_next, "__next__");

  std::shared_ptr<Value> GetField(const std::string& name)
  {
    return fields.at(name);
  }
  std::shared_ptr<Value> SetField(const std::string& name, VPtr value)
  {
    fields[name] = std::move(value);
    return fields.at(name);
  }

	[[nodiscard]] virtual std::shared_ptr<Value> OperatorIndex(const std::shared_ptr<Value>& idx)
	{
		throw std::runtime_error("Unsupported operator!\n");
	}
	virtual void OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val)
	{
		throw std::runtime_error("Unsupported operator!\n");
	}

	virtual std::vector<std::byte> Serialize()
  {
	  throw std::runtime_error("Value not serializable\n");
  }

private:
    VPtr dispatch(unop_fn slot, const char* opname);
    VPtr dispatch(binop_fn slot, const VPtr& rhs, const char* opname);
};

static VPtr NotImplemented{};

void init_base_methods(TypeObject* tp);


std::tuple<std::shared_ptr<ArrayValue>, std::shared_ptr<DictValue>> get_args_kwargs(ByteCodeVM& VM);
}
