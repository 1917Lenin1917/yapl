//
// Created by lenin on 09.04.2025.
//

#pragma once

#include <exception>
#include <string>
#include <utility>
#include <format>
#include <span>
#include <vector>

#include "yapl/Position.hpp"

namespace yapl {

class SyntaxError : public std::exception
{
public:
  SyntaxError(
    std::string filename,
    const Range &range,
    const std::span<const std::string> sourceLines,
    std::string description = ""
  )
    : m_Filename(std::move(filename)),
      m_Range(range),
      m_Description(std::move(description))
  {
    CopyRelevantLines(sourceLines);
    BuildMessage();
  }

  [[nodiscard]] const char* what() const noexcept override
  {
    return m_WhatBuffer.c_str();
  }

private:
  std::string m_Filename;
  Range m_Range;
  std::string m_Description;
  std::vector<std::string> m_RelevantLines;
  std::size_t m_FirstStoredLine = 1;
  std::string m_WhatBuffer;

  void CopyRelevantLines(std::span<const std::string> sourceLines);
  void BuildMessage();
};

}
