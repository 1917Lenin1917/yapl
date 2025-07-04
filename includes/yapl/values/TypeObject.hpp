//
// Created by lenin on 29.04.2025.
//

#pragma once

#include <string>
#include <unordered_map>
#include <functional>
#include <memory>

namespace yapl {

class FunctionASTNode;
class Value;
class Visitor;
class BaseASTNode;

using VPtr = std::shared_ptr<Value>;

using make_fn = std::function<VPtr(const std::vector<VPtr>& args)>;
using call_fn = std::function<VPtr(Visitor& visitor, const VPtr& self, const std::vector<std::unique_ptr<BaseASTNode>>& args)>;
using hash_fn = std::function<std::size_t(const VPtr&)>;
using unop_fn = std::function<VPtr(const VPtr&)>;
using binop_fn = std::function<VPtr(const VPtr&, const VPtr&)>;

struct TypeObject
{
    std::string name;

    std::vector<std::unique_ptr<FunctionASTNode>> methods;
    std::unordered_map<std::string, FunctionASTNode*> method_dict;

    // make_fn nb_make = [](const std::vector<VPtr>& args) -> VPtr // default impl looks for user-defined init method
    // {
    //     return nullptr;
    // };

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
    binop_fn nb_nq  = nullptr;    // ==

    unop_fn nb_str  = nullptr;    // __str__
    unop_fn nb_iter = nullptr;    // __iter__
    unop_fn nb_next = nullptr;    // __next__


    void AddMethod(const std::string& method_name, std::unique_ptr<FunctionASTNode>&& function)
    {
        methods.push_back(std::move(function));
        method_dict[method_name] = methods.back().get();
    }
};

}
