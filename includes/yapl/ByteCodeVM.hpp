//
// Created by Максим Литвиненко on 25.08.2025.
//

#pragma once
#include <stack>
#include <utility>
#include <vector>

#include "ByteCode.hpp"
#include "values/ArrayValue.hpp"
#include "values/DictValue.hpp"
#include "values/FunctionValue.hpp"
#include "values/Value.hpp"
#include "values/IntegerValue.hpp"
#include "values/TypeObjectValue.hpp"

namespace yapl {

class ByteCodeVM
{
public:
  explicit ByteCodeVM(CodeObject obj)
    : m_CodeObject(obj)
  {


    // m_Constants.push_back(mk_int(34));
    // m_Constants.push_back(mk_int(35));
    // m_Constants.push_back(mk_int(1000000));
    //
    // // m_Constants.push_back(mk_int(3));
    // m_Constants.push_back(mk_int(1));
    // m_Locals.push_back(std::make_shared<Variable>(false, VALUE_TYPE::INTEGER, mk_int(0)));
    // m_Locals.push_back(std::make_shared<Variable>(false, VALUE_TYPE::INTEGER, mk_int(0)));
  }


  void Run();


private:
  std::size_t m_Idx = 0;
  std::stack<std::shared_ptr<Value>> m_Stack;
  CodeObject m_CodeObject;

private:
  void HandleBinaryOp(BinaryOp compare_type);
};

}
