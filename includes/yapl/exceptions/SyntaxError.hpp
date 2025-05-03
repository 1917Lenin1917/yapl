//
// Created by lenin on 09.04.2025.
//

#pragma once

#include <exception>
#include <string>
#include <utility>
#include <format>

namespace yapl {
class SyntaxError : public std::exception
{
private:
    std::string m_filename;
    int line, col_start, col_end;
    std::string source_line;
    std::string description;

    std::string what_buffer;

    void build_message();
public:
    SyntaxError(std::string filename, int line, int col_start, int col_end, std::string source_line, std::string description = "")
        :m_filename(std::move(filename)), line(line), col_start(col_start), col_end(col_end), source_line(std::move(source_line)), description(std::move(description))
    {
        build_message();
    }

    [[nodiscard]] const char * what() const noexcept override
    {
        return what_buffer.c_str();
    }
};

}
