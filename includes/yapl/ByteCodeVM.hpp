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
#include "values/Value.hpp"
#include "values/BuiltinFunctionValue.hpp"
#include "values/IntegerValue.hpp"

namespace yapl {

struct Frame
{
  std::shared_ptr<CodeObject> code_object;
  std::vector<std::shared_ptr<Variable>> locals;
};

class ByteCodeVM
{
public:
  explicit ByteCodeVM(CodeObject obj)
    : m_CodeObject(std::move(obj))
  {
    std::vector<std::shared_ptr<Variable>> locals;
    locals.reserve(m_CodeObject.locals.size());
    for (const auto& name : m_CodeObject.locals)
    {
      locals.push_back(std::make_shared<Variable>(true, VALUE_TYPE::UNDEFINED, nullptr, "__main__", name));
    }
    Frame frame = {
      .code_object = std::make_shared<CodeObject>(m_CodeObject),
      .locals = locals
    };
    m_FrameStack.push_back(frame);

    auto fn = mk_builtin("print", [](ByteCodeVM& VM)
    {
      const auto _args = VM.m_Stack.top();
      VM.m_Stack.pop();
      const auto args = static_cast<ArrayValue*>(_args.get());

      for (const auto value : args->value)
      {
        std::cout << value->print() << " ";
      }
      std::cout << "\n";
    });

    m_Globals["print"] = std::make_shared<Variable>(true, VALUE_TYPE::BUILTIN_FUNCTION, fn, "__main__", "print", false);
  }


  void Run();
  void Run(CodeObject& co);


private:
  std::size_t m_Idx = 0;
  std::stack<std::shared_ptr<Value>> m_Stack;
  std::vector<Frame> m_FrameStack;
  CodeObject m_CodeObject;
  std::unordered_map<std::string, std::shared_ptr<Variable>> m_Globals;

private:
  void HandleBinaryOp(BinaryOp compare_type);
};

}
