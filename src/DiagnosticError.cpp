#include "yapl/exceptions/DiagnosticError.hpp"

#include <algorithm>

namespace yapl {

DiagnosticError::DiagnosticError(
  std::string filename,
  const Diagnostic& diagnostic,
  std::span<const std::string> sourceLines
)
  : m_Filename(std::move(filename)),
    m_Diagnostic(diagnostic)
{
  CopyRelevantLines(sourceLines);
  BuildMessage();
}

const char* DiagnosticError::what() const noexcept
{
  return m_WhatBuffer.c_str();
}

const Diagnostic& DiagnosticError::GetDiagnostic() const noexcept
{
  return m_Diagnostic;
}

void DiagnosticError::CopyRelevantLines(const std::span<const std::string> sourceLines)
{
  if (sourceLines.empty())
  {
    m_FirstStoredLine = 0;
    return;
  }

  const Range& range = m_Diagnostic.location.range;

  const std::size_t startLine = std::min(range.start.line, sourceLines.size() - 1);
  const std::size_t endLine = std::min(std::max(range.end.line, startLine), sourceLines.size() - 1);

  m_FirstStoredLine = startLine;
  m_RelevantLines.reserve(endLine - startLine + 1);

  for (std::size_t lineIndex = startLine; lineIndex <= endLine; ++lineIndex)
  {
    m_RelevantLines.emplace_back(sourceLines[lineIndex]);
  }
}

void DiagnosticError::BuildMessage()
{
  m_WhatBuffer.clear();

  const Range& range = m_Diagnostic.location.range;

  const std::size_t startLine = range.start.line;
  const std::size_t endLine = range.end.line;
  const std::size_t startColumn = range.start.character;
  const std::size_t endColumn = range.end.character;

  m_WhatBuffer += std::format(
    "File \"{}\", line {}:{} to {}:{}\n",
    m_Filename,
    startLine + 1,
    startColumn + 1,
    endLine + 1,
    endColumn + 1
  );

  m_WhatBuffer += std::format(
    "{}[{}]: {}\n",
    to_string(m_Diagnostic.severity),
    to_string(m_Diagnostic.code),
    m_Diagnostic.message
  );

  if (m_RelevantLines.empty())
  {
    return;
  }

  for (std::size_t i = 0; i < m_RelevantLines.size(); ++i)
  {
    const std::size_t absoluteLine = m_FirstStoredLine + i;
    const std::string_view sourceLine = m_RelevantLines[i];
    const std::size_t lineLength = sourceLine.size();

    std::size_t highlightStart = 0;
    std::size_t highlightEnd = lineLength > 0 ? lineLength : 1;

    if (absoluteLine == startLine)
    {
      highlightStart = std::min(startColumn, lineLength);
    }

    if (absoluteLine == endLine)
    {
      highlightEnd = std::min(std::max(endColumn, highlightStart + 1), lineLength > 0 ? lineLength : 1);
    }

    if (startLine == endLine)
    {
      highlightStart = std::min(startColumn, lineLength);
      highlightEnd = std::min(std::max(endColumn, highlightStart + 1), lineLength > 0 ? lineLength : 1);
    }

    if (lineLength == 0)
    {
      highlightStart = 0;
      highlightEnd = 1;
    }
    else if (highlightEnd <= highlightStart)
    {
      highlightEnd = std::min(highlightStart + 1, lineLength);
      if (highlightEnd <= highlightStart)
      {
        highlightStart = std::min(highlightStart, lineLength - 1);
        highlightEnd = highlightStart + 1;
      }
    }

    m_WhatBuffer += std::format("{:>4} | ", absoluteLine + 1);
    m_WhatBuffer += sourceLine;
    m_WhatBuffer += '\n';

    m_WhatBuffer += "     | ";
    m_WhatBuffer += std::string(highlightStart, ' ');
    m_WhatBuffer += std::string(highlightEnd - highlightStart, '^');
    m_WhatBuffer += '\n';
  }
}

}
