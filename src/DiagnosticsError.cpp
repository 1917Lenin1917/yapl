#include "yapl/exceptions/DiagnosticsError.hpp"

#include <format>

namespace yapl {

DiagnosticsError::DiagnosticsError(
  std::string filename,
  std::span<const Diagnostic> diagnostics,
  std::span<const std::string> sourceLines
)
{
  m_Errors.reserve(diagnostics.size());

  for (const Diagnostic& diagnostic : diagnostics)
  {
    m_Errors.emplace_back(filename, diagnostic, sourceLines);
  }

  BuildMessage();
}

const char* DiagnosticsError::what() const noexcept
{
  return m_WhatBuffer.c_str();
}

const std::vector<DiagnosticError>& DiagnosticsError::GetErrors() const noexcept
{
  return m_Errors;
}

void DiagnosticsError::BuildMessage()
{
  m_WhatBuffer.clear();

  m_WhatBuffer += std::format("Resolver produced {} diagnostic(s)\n", m_Errors.size());

  for (std::size_t i = 0; i < m_Errors.size(); ++i)
  {
    m_WhatBuffer += '\n';
    m_WhatBuffer += std::format("[{}]\n", i + 1);
    m_WhatBuffer += m_Errors[i].what();
  }
}

[[noreturn]] void ThrowDiagnosticsError(
  std::string filename,
  std::span<const Diagnostic> diagnostics,
  std::span<const std::string> sourceLines
)
{
  throw DiagnosticsError(std::move(filename), diagnostics, sourceLines);
}

void ThrowIfHasDiagnosticErrors(
  std::string filename,
  std::span<const Diagnostic> diagnostics,
  std::span<const std::string> sourceLines
)
{
  std::size_t errorCount = 0;

  for (const Diagnostic& diagnostic : diagnostics)
  {
    if (diagnostic.severity == DiagnosticSeverity::ERROR)
    {
      ++errorCount;
    }
  }

  if (errorCount == 0)
  {
    return;
  }

  std::vector<Diagnostic> onlyErrors;
  onlyErrors.reserve(errorCount);

  for (const Diagnostic& diagnostic : diagnostics)
  {
    if (diagnostic.severity == DiagnosticSeverity::ERROR)
    {
      onlyErrors.push_back(diagnostic);
    }
  }

  throw DiagnosticsError(std::move(filename), onlyErrors, sourceLines);
}

}
