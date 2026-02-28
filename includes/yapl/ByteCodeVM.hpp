//
// Created by Максим Литвиненко on 25.08.2025.
//

#pragma once
#include <filesystem>
#include <stack>
#include <utility>
#include <vector>
#include <memory>
#include <unordered_map>

#include "ByteCode.hpp"
#include "Variable.hpp"
#include "values/ArrayValue.hpp"
#include "values/DictValue.hpp"
#include "values/Value.hpp"
#include "values/BuiltinFunctionValue.hpp"
#include "values/FunctionValue.hpp"
#include "values/IntegerValue.hpp"
#include "values/TypeObjectValue.hpp"

namespace yapl {

struct Frame
{
  std::shared_ptr<CodeObject> code_object;
  std::vector<std::shared_ptr<Variable>> locals;
  std::unordered_map<std::string, std::shared_ptr<Variable>> names;

  std::shared_ptr<Frame> globals;
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

    auto frame = std::make_shared<Frame>();
    frame->code_object = std::make_shared<CodeObject>(m_CodeObject);
    frame->locals = std::move(locals);
    frame->globals = frame;

    m_FrameStack.push_back(frame);

    auto print_lambda = [](ByteCodeVM& VM)
    {
      std::string sep = " ";
      std::string end = "\n";

      const auto _args_or_kw = VM.m_Stack.top();
      if (const auto kw = dynamic_cast<DictValue*>(_args_or_kw.get()))
      {
        VM.m_Stack.pop();
        const auto sep_str = mk_str("sep");
        const auto end_str = mk_str("end");

        if (kw->value.contains(sep_str)) sep = static_cast<StringValue*>(kw->value.at(sep_str).get())->value;
        if (kw->value.contains(end_str)) end = static_cast<StringValue*>(kw->value.at(end_str).get())->value;
      }
      const auto _args = VM.m_Stack.top();
      VM.m_Stack.pop();
      const auto args = static_cast<ArrayValue*>(_args.get());

      for (std::size_t i = 0; i < args->value.size(); i++)
      {
        std::cout << args->value[i]->print();
        if (i != args->value.size() - 1) std::cout << sep;
      }
      std::cout << end;
    };
    auto fn = mk_builtin("print", print_lambda);

    m_Globals["print"] = std::make_shared<Variable>(true, VALUE_TYPE::BUILTIN_FUNCTION, fn, "__main__", "print", false);
    m_Globals[IntegerTypeObject->name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE,  mk_type(IntegerTypeObject), "__main__", IntegerTypeObject->name, false);
    m_Globals[FloatTypeObject->name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE,  mk_type(FloatTypeObject), "__main__", FloatTypeObject->name, false);
    m_Globals[ArrayTypeObject->name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE,  mk_type(ArrayTypeObject), "__main__", ArrayTypeObject->name, false);
    m_Globals[BooleanTypeObject->name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE,  mk_type(BooleanTypeObject), "__main__", BooleanTypeObject->name, false);
    m_Globals[StringTypeObject->name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE,  mk_type(StringTypeObject), "__main__", StringTypeObject->name, false);
    m_Globals[TypeObjectTypeObject->name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE,  mk_type(TypeObjectTypeObject), "__main__", TypeObjectTypeObject->name, false);
    m_Globals[DictTypeObject->name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE,  mk_type(DictTypeObject), "__main__", DictTypeObject->name, false);
    m_Globals[FunctionTypeObject->name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE,  mk_type(FunctionTypeObject), "__main__", FunctionTypeObject->name, false);
    m_Globals[SizeIteratorTypeObject->name] = std::make_shared<Variable>(true, VALUE_TYPE::TYPE,  mk_type(SizeIteratorTypeObject), "__main__", SizeIteratorTypeObject->name, false);
  }

  ~ByteCodeVM()
  {
    for (auto type : m_Types)
      delete type;
  }

  void Run();
  void Run(CodeObject& co);

  std::size_t m_Idx = 0;
  std::stack<std::shared_ptr<Value>> m_Stack;
  std::vector<std::shared_ptr<Frame>> m_FrameStack;
  CodeObject m_CodeObject;
  std::unordered_map<std::string, std::shared_ptr<Variable>> m_Globals;
  std::vector<TypeObject*> m_Types;

  std::unordered_map<std::string, std::shared_ptr<Frame>> modules;

  std::filesystem::path base_path;

private:
  void HandleUnaryOp(UnaryOp compare_type);
  void HandleBinaryOp(BinaryOp compare_type);

  void InvokeFunction(
      const VPtr& function_object,
      std::vector<VPtr> positional_arguments,
      std::unordered_map<std::string, VPtr> keyword_arguments = {});

  void LoadModule(const std::string& module_name);
};

}
