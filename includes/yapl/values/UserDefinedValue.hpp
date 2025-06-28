//
// Created by lenin on 18.05.2025.
//

#pragma once
#include "Value.hpp"

namespace yapl {

class UserDefinedValue final : public Value
{
public:
  explicit UserDefinedValue(TypeObject* tp);

  std::unique_ptr<Value> Copy() const override;
};

};
