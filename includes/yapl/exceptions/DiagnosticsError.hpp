#pragma once

#include <exception>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "yapl/exceptions/DiagnosticError.hpp"
#include "yapl/Resolver.hpp"

namespace yapl {

class DiagnosticsError : public std::exception
{
public:
  DiagnosticsError(
    std::string filename,
    std::span<const Diagnostic> diagnostics,
    std::span<const std::string> sourceLines
  );

  [[nodiscard]] const char* what() const noexcept override;

  [[nodiscard]] const std::vector<DiagnosticError>& GetErrors() const noexcept;

private:
  std::vector<DiagnosticError> m_Errors;
  std::string m_WhatBuffer;

  void BuildMessage();
};

[[noreturn]] void ThrowDiagnosticsError(
  std::string filename,
  std::span<const Diagnostic> diagnostics,
  std::span<const std::string> sourceLines
);

void ThrowIfHasDiagnosticErrors(
  std::string filename,
  std::span<const Diagnostic> diagnostics,
  std::span<const std::string> sourceLines
);

}
