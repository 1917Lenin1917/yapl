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

    void build_message()
    {
        // Build a user-friendly message
        // Note: If line_ is zero-based internally, you might display line_ + 1 to the user.
        // We'll also clamp col_start_/col_end_ if needed.
        if (col_start < 0) col_start = 0;
        if (col_end < col_start) col_end = col_start;

        // 1) Basic heading
        what_buffer += std::format("File \"{}\", line {}, col {}:{}\n", m_filename, line, col_start, col_end);

        // 2) Add the description
        what_buffer += std::format("SyntaxError: {}\n", description);

        // 3) Show the source line itself
        what_buffer += source_line;
        what_buffer += "\n";

        // 4) Show a caret or tildes to highlight the exact error range
        // We'll clamp col_end to the line length
        if (col_end > (int)source_line.size()) {
            col_end = (int)source_line.size();
        }
        // Print spaces up to col_start, then '^' or a run of '^' or '~'
        // We'll do a run of '^' for multi-character errors
        // first, col_start spaces:
        what_buffer += std::string(col_start-1, ' ');

        int highlightLen = col_end - col_start + 1;
        if (highlightLen <= 0) highlightLen = 1;  // at least one marker
        what_buffer += std::string(highlightLen, '^');
        what_buffer += "\n";
    }
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
