//
// Created by lenin on 29.04.2025.
//

#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>

namespace yapl {
class ArrayValue;
}

namespace yapl {
class ByteCodeVM;
}

namespace yapl {

class FunctionASTNode;
class Value;
class Visitor;
class BaseASTNode;

using VPtr = std::shared_ptr<Value>;

using make_fn = std::function<VPtr(const std::vector<VPtr>& args)>;
using call_fn = std::function<VPtr(ByteCodeVM& VM, const VPtr& self)>;
using hash_fn = std::function<std::size_t(const VPtr&)>;
using unop_fn = std::function<VPtr(const VPtr&)>;
using binop_fn = std::function<VPtr(const VPtr&, const VPtr&)>;

struct TypeObject
{
    std::string name;

    std::unordered_map<std::string, std::shared_ptr<Value>> methods;

    make_fn nb_make = nullptr;

    hash_fn nb_hash = nullptr;

    unop_fn nb_pos  = nullptr;    // + (unary)
    unop_fn nb_neg  = nullptr;    // - (unary)
    unop_fn nb_not  = nullptr;    // ! (unary)
    call_fn nb_call = nullptr;

    binop_fn nb_add = nullptr;    // +
    binop_fn nb_sub = nullptr;    // -
    binop_fn nb_mul = nullptr;    // *
    binop_fn nb_div = nullptr;    // /
    binop_fn nb_mod = nullptr;    // %


    binop_fn nb_lt  = nullptr;    // <
    binop_fn nb_gt  = nullptr;    // >
    binop_fn nb_le  = nullptr;    // <=
    binop_fn nb_ge  = nullptr;    // >=
    binop_fn nb_eq  = nullptr;    // ==
    binop_fn nb_nq  = nullptr;    // !=
    binop_fn nb_and = nullptr;    // and
    binop_fn nb_or  = nullptr;    // or

    unop_fn nb_str  = nullptr;    // __str__
    unop_fn nb_iter = nullptr;    // __iter__
    unop_fn nb_next = nullptr;    // __next__
};

}
