//
// Created by lenin on 19.04.2025.
//

#pragma once

#include <exception>
#include <string>
#include <format>

namespace yapl {

class RuntimeError : public std::exception
{
private:
    std::string TYPE = "RuntimeError";
    std::string m_value;

    std::string m_buffer;
public:
    RuntimeError(std::string_view value)
        :m_value(value)
    {
        m_buffer += std::format("{}: {}\n", TYPE, m_value);
    }
    [[nodiscard]] const char * what() const noexcept override
    {
        return m_buffer.c_str();
    }
};

}