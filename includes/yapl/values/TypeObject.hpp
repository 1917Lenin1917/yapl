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

using VPtr = std::shared_ptr<Value>;

using make_fn = std::function<VPtr(const std::vector<VPtr>& args)>;
using hash_fn = std::function<std::size_t(const VPtr&)>;
using unop_fn = std::function<VPtr(const VPtr&)>;
using binop_fn = std::function<VPtr(const VPtr&, const VPtr&)>;

struct TypeObject
{
    std::string name;

    std::vector<std::unique_ptr<FunctionASTNode>> methods;
    std::unordered_map<std::string, FunctionASTNode*> method_dict;

    make_fn nb_make = nullptr;

    hash_fn nb_hash = nullptr;

    unop_fn nb_pos = nullptr;    // + (unary)
    unop_fn nb_neg = nullptr;    // - (unary)
    unop_fn nb_not = nullptr;    // ! (unary)

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


    void AddMethod(const std::string& method_name, std::unique_ptr<FunctionASTNode>&& function)
    {
        methods.push_back(std::move(function));
        method_dict[method_name] = methods.back().get();
    }
};

}
