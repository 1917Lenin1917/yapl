//
// Created by Максим Литвиненко on 28.08.2025.
//

#pragma once
#include "Value.hpp"


namespace yapl {

class UndefinedValue final: public Value
{
public:
  explicit UndefinedValue();

  bool IsTruthy() const override { return false; }

  std::unique_ptr<Value> Copy() const override { return nullptr; }


};


inline TypeObject* UndefinedTypeObject = nullptr;

static void init_undefined_tp()
{
  UndefinedTypeObject = new TypeObject{ .name = "Undefined" };
}

}
