//
// Created by lenin on 29.06.2025.
//

#include <utility>

#include "yapl/values/SizeIterator.hpp"

namespace yapl {

  SizeIteratorValue::SizeIteratorValue(const std::shared_ptr<Value>& iterable, std::shared_ptr<IntegerValue> len)
    : Value(VALUE_TYPE::ITERATOR, SizeIteratorTypeObject), iterable(iterable), idx(std::make_shared<IntegerValue>(0)), len(std::move(len)) {}
}
