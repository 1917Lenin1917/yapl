//
// Created by lenin on 26.02.2026.
//


#include <utility>

#include "yapl/values/ModuleValue.hpp"

namespace yapl {

ModuleValue::ModuleValue(std::shared_ptr<Frame> module)
  :Value(VALUE_TYPE::MODULE, ModuleTypeObject), module(std::move(module)) {  }

}