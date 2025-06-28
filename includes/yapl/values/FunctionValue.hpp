//
// Created by lenin on 18.05.2025.
//

#pragma once
#include "Value.hpp"
#include "StringValue.hpp"

namespace yapl {

class FunctionASTNode;

class FunctionValue final : public Value
{
public:
  std::string name;
  FunctionASTNode* fn;

  explicit FunctionValue(std::string name, FunctionASTNode* fn);

  std::unique_ptr<Value> Copy() const override;
};

inline TypeObject* FunctionTypeObject = nullptr;

void init_function_methods(TypeObject* tp);

static void init_function_tp()
{
  FunctionTypeObject = new TypeObject{"function"};

  FunctionTypeObject->nb_str = [](const VPtr& self)
  {
    auto fn = as_func(self.get())->fn;

    auto decl = static_cast<FunctionDeclASTNode*>(fn->decl.get());
    return mk_str(decl->name.value);
  };

  init_base_methods(FunctionTypeObject);
  init_function_methods(FunctionTypeObject);

}

}
