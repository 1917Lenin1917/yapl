//
// Created by lenin on 18.05.2025.
//

#pragma once
#include "Value.hpp"

namespace yapl {

class FunctionASTNode;

class FunctionValue final : public Value
{
public:
  std::string name;
  FunctionASTNode* fn;

  explicit FunctionValue(std::string name, FunctionASTNode* fn);

  std::string print() const override;
  std::unique_ptr<Value> Copy() const override;
};

inline TypeObject* FunctionTypeObject = nullptr;

void init_function_methods(TypeObject* tp);

static void init_function_tp()
{
  FunctionTypeObject = new TypeObject{"function"};

  init_base_methods(FunctionTypeObject);
  init_function_methods(FunctionTypeObject);

}

}
