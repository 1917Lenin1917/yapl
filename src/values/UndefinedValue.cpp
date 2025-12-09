//
// Created by Максим Литвиненко on 28.08.2025.
//

#include "../../includes/yapl/values/UndefinedValue.hpp"

namespace yapl {

UndefinedValue::UndefinedValue()
  :Value(VALUE_TYPE::UNDEFINED, UndefinedTypeObject)
{

}
}
