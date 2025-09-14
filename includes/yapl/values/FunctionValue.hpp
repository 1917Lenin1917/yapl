//
// Created by lenin on 18.05.2025.
//

#pragma once
#include "Value.hpp"
#include "StringValue.hpp"

namespace yapl {

class FunctionValue final : public Value
{
public:
  std::shared_ptr<CodeObject>& code_object;
  std::string name;

  explicit FunctionValue(std::string name, std::shared_ptr<CodeObject>& code_object);

  std::unique_ptr<Value> Copy() const override;
};

inline TypeObject* FunctionTypeObject = nullptr;

void init_function_methods(TypeObject* tp);

static void init_function_tp()
{
  FunctionTypeObject = new TypeObject{"function"};

  FunctionTypeObject->nb_str = [](const VPtr& self)
  {
    auto name = as_func(self.get())->name;
    return mk_str(name);
  };

  init_base_methods(FunctionTypeObject);
  init_function_methods(FunctionTypeObject);

}

}
