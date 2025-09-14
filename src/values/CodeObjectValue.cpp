//
// Created by Максим Литвиненко on 11.09.2025.
//

#include "yapl/values/CodeObjectValue.hpp"


namespace yapl {

CodeObjectValue::CodeObjectValue(const std::shared_ptr<CodeObject>& code_object)
  :Value(VALUE_TYPE::CODE_OBJECT, CodeObjectTypeObject), code_object(code_object) {  }

}
