//
// Created by lenin on 02.12.2024.
//

#include "yapl/values/Value.hpp"
#include "yapl/values/StringValue.hpp"

namespace yapl {
    std::string value_type_to_string(VALUE_TYPE vt)
    {
        switch (vt) {
            case VALUE_TYPE::STRING: return "str";
            default: return "unhandled";
        }
    }

void Value::make_type()
{
    const Token name{ TOKEN_TYPE::IDENTIFIER, new char[]("type")  };
    std::vector<std::unique_ptr<FunctionArgumentASTNode>> arg_list;
    const Token return_type{ TOKEN_TYPE::IDENTIFIER, new char[]("str") };
    auto body = std::make_unique<BuiltinCustomVisitFunctionASTNode>([&](std::shared_ptr<Function> f_obj)
    {
        return std::make_unique<StringValue>(value_type_to_string(this->type));
    });

    auto f = std::make_unique<FunctionASTNode>(
            std::move(std::make_unique<FunctionDeclASTNode>(name, std::make_unique<FunctionArgumentListASTNode>(arg_list, -1), return_type)),
            std::move(body));
    m_methods.push_back(std::move(f));
    m_method_definitions["type"] = m_methods[m_methods.size() - 1].get();
}

Value::Value(const VALUE_TYPE type = VALUE_TYPE::NONE)
	:type(type)
{
    make_type();
}

}
