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
#include "values/Value.hpp"
#include "values/IntegerValue.hpp"

namespace yapl {
struct CodeObject;

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
  ByteCodeVM(const std::filesystem::path& path);

  ~ByteCodeVM()
  {
    for (const auto type : m_Types)
      delete type;
  }

  void Run();
  void Run(const CodeObject& co);

  std::size_t m_Idx = 0;
  std::stack<std::shared_ptr<Value>> m_Stack;
  std::vector<std::shared_ptr<Frame>> m_FrameStack;
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
