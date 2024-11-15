//
// Created by lenin on 15.11.2024.
//

#pragma once
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>

namespace yapl {
class BaseASTNode;

struct Variable
{
  bool is_constant;
  int value;
};

struct Function 
{
  std::unordered_map<std::string, std::unique_ptr<Variable>> args;
	std::vector<std::string> arg_names; // needed for correct order
  std::unordered_map<std::string, std::unique_ptr<Variable>> vars;

	std::unique_ptr<BaseASTNode> ast;
};

class Interpreter {
private:
public:
  std::unordered_map<std::string, std::unique_ptr<Variable>> vars;
	std::unordered_map<std::string, std::unique_ptr<Function>> functions;
	std::string active_func;
};

class Visitor
{
private:
public:
  Interpreter& interpreter;
  Visitor(Interpreter& i)
    :interpreter(i) 
	{
		// set built-in functions 
		// TODO:
	}

};

}



