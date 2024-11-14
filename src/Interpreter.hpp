//
// Created by lenin on 15.11.2024.
//

#pragma once
#include <memory>
#include <unordered_map>

namespace yapl {

struct Variable
{
  bool is_constant;
  int value;
};

class Interpreter {
private:
  // std::unique_ptr<BaseASTNode> m_root; // Should it be a pointer to root node?
public:
  std::unordered_map<std::string, std::unique_ptr<Variable>> vars;
  // Interpreter(std::unique_ptr<BaseASTNode> root)
  //   :m_root(std::move(root)) {}

  // void interpret_var_decl(const std::unique_ptr<VariableASTNode>& decl_node)
  // {
  //   // check if var already exists
  //   if (m_vars.contains(decl_node->name.value))
  //   {
  //     std::cout << "Var " << decl_node->name.value << " already declared!\n";
  //     return;
  //   }
  //   int value = 0; // change later
  //   if (decl_node->value)
  //     value = evaluate_expr(decl_node->value);
  //   m_vars[decl_node->name.value] = std::make_unique<Variable>{ decl_node->type.type == TOKEN_TYPE::CONST, value };
  // }

};

class Visitor
{
private:
public:
  Interpreter& interpreter;
  Visitor(Interpreter& i)
    :interpreter(i) {}

};

}



