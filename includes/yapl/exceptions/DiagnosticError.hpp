#pragma once

#include <exception>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "yapl/Position.hpp"
#include "yapl/Resolver.hpp"

namespace yapl {

class DiagnosticError : public std::exception
{
public:
  DiagnosticError(
    std::string filename,
    const Diagnostic& diagnostic,
    std::span<const std::string> sourceLines
  );

  [[nodiscard]] const char* what() const noexcept override;

  [[nodiscard]] const Diagnostic& GetDiagnostic() const noexcept;

private:
  std::string m_Filename;
  Diagnostic m_Diagnostic;
  std::vector<std::string> m_RelevantLines;
  std::size_t m_FirstStoredLine = 0;
  std::string m_WhatBuffer;

  void CopyRelevantLines(std::span<const std::string> sourceLines);
  void BuildMessage();
};

}
