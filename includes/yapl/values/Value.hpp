//
// Created by lenin on 02.12.2024.
//

#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>

#include "yapl/ASTNode.hpp"

namespace yapl {
class FunctionASTNode;

enum class VALUE_TYPE
{
	NONE = 0,
	INTEGER,
	FLOAT,
	BOOL,
	STRING,
	ARRAY,
};

class Value
{
private:
    void make_type();
protected:
	std::vector<std::unique_ptr<FunctionASTNode>> m_methods;
	std::unordered_map<std::string, FunctionASTNode*> m_method_definitions;
public:
	VALUE_TYPE type;

	explicit Value(VALUE_TYPE type);
	virtual ~Value() = default;

	[[nodiscard]] virtual std::string print() const = 0;
	[[nodiscard]] virtual std::unique_ptr<Value> Copy() const = 0;
	virtual void Set(const std::shared_ptr<Value>& v) { throw std::runtime_error("Not implemented"); }

	virtual std::shared_ptr<Value> UnaryMinus() = 0;
	virtual std::shared_ptr<Value> UnaryPlus() = 0;
	virtual std::shared_ptr<Value> UnaryNot() = 0;


	virtual std::shared_ptr<Value> BinaryPlus(const std::shared_ptr<Value>& other) = 0;
	virtual std::shared_ptr<Value> BinaryTimes(const std::shared_ptr<Value>& other) = 0;
	virtual std::shared_ptr<Value> BinaryMinus(const std::shared_ptr<Value>& other) = 0;
	virtual std::shared_ptr<Value> BinarySlash(const std::shared_ptr<Value>& other) = 0;
    virtual std::shared_ptr<Value> BinaryMOD(const std::shared_ptr<Value>& other) = 0;
	virtual std::shared_ptr<Value> BinaryLT(const std::shared_ptr<Value>& other) = 0;
	virtual std::shared_ptr<Value> BinaryGT(const std::shared_ptr<Value>& other) = 0;
	virtual std::shared_ptr<Value> BinaryLQ(const std::shared_ptr<Value>& other) = 0;
	virtual std::shared_ptr<Value> BinaryGQ(const std::shared_ptr<Value>& other) = 0;
	virtual std::shared_ptr<Value> BinaryEQ(const std::shared_ptr<Value>& other) = 0;

	[[nodiscard]] virtual std::shared_ptr<Value> OperatorIndex(const std::shared_ptr<Value>& idx)
	{
		throw std::runtime_error("Unsupported operator!\n");
	}
	virtual void OperatorIndexSet(const std::shared_ptr<Value> &idx, std::shared_ptr<Value> new_val)
	{
		throw std::runtime_error("Unsupported operator!\n");
	}

	[[nodiscard]] FunctionASTNode* get_method_definition(const std::string& name)
	{
		if (m_method_definitions.contains(name))
			return m_method_definitions[name];

		std::cerr << "Unknown method " << name << "\n";
		return nullptr;
	}
};

}
