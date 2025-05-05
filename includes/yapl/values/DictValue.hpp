//
// Created by lenin on 04.05.2025.
//

#pragma once
#include <yapl/exceptions/RuntimeError.hpp>

#include "Value.hpp"

namespace yapl {

struct DictHash
{
  std::size_t operator()(const VPtr& self) const
  {
    if(const auto& hash_func = self->tp->nb_hash)
    {
      return hash_func(self);
    }
    throw RuntimeError(std::format("Unhashable type: {}", self->tp->name));
  }

};

struct DictEq
{
  bool operator()(const VPtr& self, const VPtr& other) const
  {
    if (auto f = self->tp->nb_eq)
    {
      auto v = f(self, other);
      if (v != NotImplemented)
        return v->IsTruthy();
    }
    if (auto f = other->tp->nb_eq)
    {
      auto v = f(other, self);
      if (v != NotImplemented)
        return v->IsTruthy();
    }
    throw RuntimeError(std::format("Unsupported operand types for ==: '{}' and '{}'", value_type_to_string(self->type), value_type_to_string(other->type)));
  }
};

class DictValue final : public Value
{
public:
  std::unordered_map<VPtr, VPtr, DictHash, DictEq> value;

  explicit DictValue();

  [[nodiscard]] std::string print() const override;
  [[nodiscard]] std::unique_ptr<Value> Copy() const override;

  [[nodiscard]] std::shared_ptr<Value> OperatorIndex(const std::shared_ptr<Value> &idx) override;
  void OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val) override;
};

inline TypeObject* DictTypeObject = nullptr;

void init_dict_methods(TypeObject* tp);

static void init_dict_tp()
{
  DictTypeObject = new TypeObject{ "dict" };

  init_base_methods(DictTypeObject);
  init_dict_methods(DictTypeObject);
}

}
