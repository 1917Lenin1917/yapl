//
// Created by lenin on 19.04.2025.
//

#pragma once

#include <exception>
#include <string>
#include <format>

namespace yapl {

class StopIteration : public std::exception
{
private:
    std::string TYPE = "StopIteration";
public:
    StopIteration() {}
    [[nodiscard]] const char * what() const noexcept override
    {
        return "";
    }
};

}