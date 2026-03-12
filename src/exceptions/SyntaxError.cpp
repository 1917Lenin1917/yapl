#include "yapl/exceptions/SyntaxError.hpp"

namespace yapl {

void SyntaxError::CopyRelevantLines(const std::span<const std::string> sourceLines)
{
  if (sourceLines.empty())
  {
    m_FirstStoredLine = 1;
    return;
  }

  const std::size_t startLine =
    std::clamp(m_Range.start.line, std::size_t{1}, sourceLines.size());

  const std::size_t endLine =
    std::clamp(m_Range.end.line, startLine, sourceLines.size());

  m_FirstStoredLine = startLine;
  m_RelevantLines.reserve(endLine - startLine + 1);

  for (std::size_t lineNumber = startLine; lineNumber <= endLine; ++lineNumber)
  {
    m_RelevantLines.emplace_back(sourceLines[lineNumber - 1]);
  }
}

void SyntaxError::BuildMessage()
{
  m_WhatBuffer.clear();

  const std::size_t startLine = m_Range.start.line;
  const std::size_t endLine = m_Range.end.line;
  const std::size_t startColumn = m_Range.start.character;
  const std::size_t endColumn = m_Range.end.character;

  m_WhatBuffer += std::format(
    "File \"{}\", line {}:{} to {}:{}\n",
    m_Filename,
    startLine + 1,
    startColumn + 1,
    endLine + 1,
    endColumn + 1
  );

  if (!m_Description.empty())
  {
    m_WhatBuffer += std::format("SyntaxError: {}\n", m_Description);
  }
  else
  {
    m_WhatBuffer += "SyntaxError\n";
  }

  if (m_RelevantLines.empty())
  {
    return;
  }

  const std::size_t lineCount = m_RelevantLines.size();

  const std::size_t clampedStartLine =
    std::clamp(startLine, std::size_t{1}, lineCount);

  const std::size_t clampedEndLine =
    std::clamp(endLine, clampedStartLine, lineCount);

  for (std::size_t lineNumber = clampedStartLine; lineNumber <= clampedEndLine; ++lineNumber)
  {
    const std::string_view sourceLine = m_RelevantLines[lineNumber - 1];
    const std::size_t lineLength = sourceLine.size();

    std::size_t highlightStart = 1;
    std::size_t highlightEnd = lineLength + 1;

    if (lineNumber == clampedStartLine)
    {
      highlightStart = std::clamp(startColumn, std::size_t{1}, lineLength + 1);
    }

    if (lineNumber == clampedEndLine)
    {
      highlightEnd = std::clamp(endColumn, highlightStart, lineLength + 1);
    }

    if (clampedStartLine == clampedEndLine)
    {
      highlightStart = std::clamp(startColumn, std::size_t{1}, lineLength + 1);
      highlightEnd = std::clamp(endColumn, highlightStart, lineLength + 1);
    }

    const std::size_t caretOffset = highlightStart > 0 ? highlightStart - 1 : 0;
    const std::size_t highlightLength =
      highlightEnd > highlightStart
        ? highlightEnd - highlightStart
        : 1;

    m_WhatBuffer += std::format("{:>4} | ", lineNumber);
    m_WhatBuffer += sourceLine;
    m_WhatBuffer += '\n';

    m_WhatBuffer += "     | ";
    m_WhatBuffer += std::string(caretOffset, ' ');
    m_WhatBuffer += std::string(highlightLength + 1, '^');
    m_WhatBuffer += '\n';
  }
}

}
