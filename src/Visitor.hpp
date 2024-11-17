//
// Created by lenin on 17.11.2024.
//

#pragma once
#include "Interpreter.hpp"

namespace yapl {
class Interpreter; // forward declaration because c++ is strange

class Visitor
{
private:
public:
  yapl::Interpreter& interpreter;
  explicit Visitor(Interpreter& i)
    :interpreter(i)
	{
		// set built-in functions
		// TODO:
	}


};
}
