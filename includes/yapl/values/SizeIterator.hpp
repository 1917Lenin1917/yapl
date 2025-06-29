//
// Created by lenin on 29.06.2025.
//

#pragma once

#include <yapl/exceptions/StopIteration.hpp>

#include "IntegerValue.hpp"
#include "Value.hpp"

namespace yapl {
class SizeIteratorValue : public Value
{
public:
  std::shared_ptr<Value> iterable;
  std::shared_ptr<IntegerValue> idx, len;

  std::unique_ptr<Value> Copy() const override { return nullptr; }

	explicit SizeIteratorValue(const std::shared_ptr<Value>& iterable, std::shared_ptr<IntegerValue> len);
};

inline TypeObject* SizeIteratorTypeObject = nullptr;

static void init_size_iterator_type()
{
  SizeIteratorTypeObject = new TypeObject{"size_iter"};

  // MAKE_METHOD(SizeIteratorTypeObject, "next", "any", ARG("this", "any"))
  // {
  //   auto self = f_obj->function_scope->vars.at("this")->value;
  //   return SizeIteratorTypeObject->nb_next(self);
  // };

  SizeIteratorTypeObject->nb_make = [](const std::vector<VPtr>& args) -> VPtr
  {
    auto iterable = args[0];
    auto len_v = as_int(args[0].get())->value;
    return mk_size_iter(iterable, mk_int(len_v));
  };

  SizeIteratorTypeObject->nb_next = [](const VPtr& raw_self)
  {
    auto self = as_size_iter(raw_self.get());

    if (self->idx->value == self->len->value)
      throw StopIteration();

    auto v = self->iterable->OperatorIndex(self->idx);
    self->idx->value++;
    return v;
  };
}

}