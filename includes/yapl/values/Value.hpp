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

#define mk_func(name, fn) std::make_shared<FunctionValue>(name, fn)
#define as_func(v) static_cast<FunctionValue*>(v)

struct TypeObject;

class FunctionASTNode;
class FunctionArgumentASTNode;
class FunctionArgumentListASTNode;
class BuiltinCustomVisitFunctionASTNode;
class FunctionDeclASTNode;

class ArrayValue;
class StringValue;

enum class VALUE_TYPE
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
  FUNCTION
};

std::string value_type_to_string(VALUE_TYPE vt);

template<class Class>
struct MethodAutoReg {
    Class* self;
    char*  name_lit;
    char*  ret_lit;
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> args;
    std::unique_ptr<FunctionArgumentASTNode> args_arg = nullptr;

    template<class Body>
    void operator=(Body&& user_body) {
        /* build the AST for the body the user just wrote */
        auto body_node =
                std::make_unique<BuiltinCustomVisitFunctionASTNode>(
                        std::forward<Body>(user_body));

        /* tokens */
        const Token name_tok  { TOKEN_TYPE::IDENTIFIER, name_lit };
        const Token ret_tok   { TOKEN_TYPE::IDENTIFIER, ret_lit };

        /* full FunctionASTNode */
        auto func = std::make_unique<FunctionASTNode>(
                std::make_unique<FunctionDeclASTNode>(
                        name_tok,
                        std::make_unique<FunctionArgumentListASTNode>(
                                std::move(args), std::move(args_arg), nullptr),
                        ret_tok),
                std::move(body_node));
        self->AddMethod(name_lit, std::move(func));
    }
};

template<class... P>
std::vector<std::unique_ptr<FunctionArgumentASTNode>>
make_arg_vector(P&&... p)
{
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> v;
    v.reserve(sizeof...(P));
    (v.emplace_back(std::forward<P>(p)), ...);
    return v;
}

#define ARG(name_lit, type_lit)                                         \
    std::make_unique<FunctionArgumentASTNode>(                          \
        Token{TOKEN_TYPE::IDENTIFIER, new char[](name_lit)},            \
        Token{TOKEN_TYPE::IDENTIFIER, new char[](type_lit)})

#define VA_ARG(name_lit, type_lit)                                         \
    std::make_unique<FunctionArgumentASTNode>(                             \
        Token{TOKEN_TYPE::IDENTIFIER, new char[](name_lit)},               \
        Token{TOKEN_TYPE::IDENTIFIER, new char[](type_lit)}, true, false)

#define MAKE_METHOD(this, name_lit, ret_lit, ...)                       \
    MethodAutoReg<std::remove_pointer_t<decltype(this)>>{               \
        this, new char[](name_lit), new char[](ret_lit),                \
        make_arg_vector(__VA_ARGS__), nullptr                           \
    } = [&](std::shared_ptr<Function> f_obj)->std::shared_ptr<Value>

#define MAKE_METHOD_WITH_VARGS(this, name_lit, ret_lit, ...)            \
    MethodAutoReg<std::remove_pointer_t<decltype(this)>>{               \
        this, new char[](name_lit), new char[](ret_lit),                \
        make_arg_vector(__VA_ARGS__), VA_ARG("args", "args")            \
    } = [&](std::shared_ptr<Function> f_obj)->std::shared_ptr<Value>

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

  TypeObject* tp = nullptr;
	VALUE_TYPE type;

	explicit Value(VALUE_TYPE type, TypeObject* tp);
	virtual ~Value() = default;

  void AddMethod(const std::string& method_name, std::unique_ptr<FunctionASTNode>&& function)
  {
      tp->methods.push_back(std::move(function));
      tp->method_dict[method_name] = tp->methods.back().get();
  }

	[[nodiscard]] virtual std::string print() const = 0;
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

	[[nodiscard]] FunctionASTNode* get_method_definition(const std::string& name)
	{
		if (tp->method_dict.contains(name))
			return tp->method_dict[name];

		std::cerr << "Unknown method " << name << "\n";
		return nullptr;
	}
private:
    VPtr dispatch(unop_fn slot, const char* opname);
    VPtr dispatch(binop_fn slot, const VPtr& rhs, const char* opname);
};

static VPtr NotImplemented{};

void init_base_methods(TypeObject* tp);
}
