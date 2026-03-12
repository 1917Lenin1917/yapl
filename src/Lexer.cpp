//
// Created by lenin on 13.11.2024.
//

#include <unordered_map>

#include "yapl/Lexer.hpp"


namespace yapl {

auto Lexer::Tokenize() -> std::vector<Token>
{
  const std::size_t text_len = m_Text.length();

  while (m_Pos != text_len)
  {
    m_Pos++;
    m_ColPos++;
    if (m_Pos >= text_len)
    {
      CheckInsertSemicolon();
      if (m_Tokens.empty() || m_Tokens[m_Tokens.size()-1].type != TOKEN_TYPE::TT_EOF)
        m_Tokens.emplace_back(TOKEN_TYPE::TT_EOF);
      return std::move(m_Tokens);
    }
    switch (const char c = m_Text[m_Pos])
    {
      case '\0':
      {
        CheckInsertSemicolon();
        m_Tokens.emplace_back(TOKEN_TYPE::TT_EOF);
        break;
      }
      case '\n':
      {
        CheckInsertSemicolon();
        m_LinePos++;
        m_ColPos = -1;
        break;
      }
      case '\t':
      case '\r':
      case ' ': { continue; }
      case '+':
      {
        MakeOperator(TOKEN_TYPE::PLUS, TOKEN_TYPE::PLUSEQ);
        break;
      }
      case '-':
      {
        MakeOperator(TOKEN_TYPE::MINUS, TOKEN_TYPE::MINUSEQ);
        break;
      }
      case '*':
      {
        MakeOperator(TOKEN_TYPE::TIMES, TOKEN_TYPE::TIMESEQ);
        break;
      }
      case '%':
      {
        MakeOperator(TOKEN_TYPE::MOD, TOKEN_TYPE::MODEQ);
        break;
      }
      case '!':
      {
        MakeOperator(TOKEN_TYPE::NOT, TOKEN_TYPE::NEQ);
        break;
      }
      case '<':
      {
        MakeOperator(TOKEN_TYPE::LT, TOKEN_TYPE::LQ);
        break;
      }
      case '>':
      {
        MakeOperator(TOKEN_TYPE::GT, TOKEN_TYPE::GQ);
        break;
      }
      case '/':
      {
        if (m_Pos + 1 != text_len && m_Text[m_Pos+1] == '/')
        {
          m_Pos += 2; // eat //
          while (m_Pos != text_len && m_Text[m_Pos] != '\n')
          {
            m_Pos++; // eat everything until \n or EOF
          }
          m_LinePos++;
          m_ColPos = -1;
          break;
        }
        MakeOperator(TOKEN_TYPE::SLASH, TOKEN_TYPE::SLASHEQ);
        break;
      }
      case '.':
      {
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::PERIOD,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case '(':
      {
        m_ParenDepth++;
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::LPAREN,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case ')':
      {
        m_ParenDepth--;
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::RPAREN,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case '{':
      {
        m_LastClosedBraceWasExpression = false;

        if (m_PendingImport) {
          m_InsideImportList = true;
          m_PendingImport = false;
          m_BraceStack.push_back(BraceKind::IMPORT_LIST);
        } else if (m_PendingExport) {
          m_InsideExportList = true;
          m_PendingExport = false;
          m_BraceStack.push_back(BraceKind::EXPORT_LIST);
        } else if (!m_Tokens.empty() &&
                   (m_Tokens.back().type == TOKEN_TYPE::ASSIGN ||
                    m_Tokens.back().type == TOKEN_TYPE::RETURN)) {
            m_BraceStack.push_back(BraceKind::OBJECT);
        } else {
            m_BraceStack.push_back(BraceKind::BLOCK);
        }

        m_BraceDepth++;
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::LBRACK,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case '}':
      {
        CheckInsertSemicolon();

        auto closed_kind = BraceKind::BLOCK;
        if (!m_BraceStack.empty()) {
          closed_kind = m_BraceStack.back();
          m_BraceStack.pop_back();
        }

        if (closed_kind == BraceKind::IMPORT_LIST) {
          m_InsideImportList = false;
        } else if (closed_kind == BraceKind::EXPORT_LIST) {
          m_InsideExportList = false;
        }

        m_LastClosedBraceWasExpression = closed_kind == BraceKind::OBJECT;

        m_BraceDepth--;
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::RBRACK,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case '[':
      {
        m_SqBraceDepth++;
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::LSQBRACK,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case ']':
      {
        m_SqBraceDepth--;
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::RSQBRACK,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case ';':
      {
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::SEMICOLON,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case ':':
      {
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::COLON,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case ',':
      {
        m_Tokens.push_back(Token{
          .type = TOKEN_TYPE::COMMA,
          .range = Range{
            .start = Position{.line = m_LinePos, .character = m_Pos },
            .end = Position{.line = m_LinePos, .character = m_Pos },
          },
        });
        break;
      }
      case '\'':
      case '\"': { MakeString(); break; }
      case '`': { MakeFormatString(); break; }
      case '=':
      {
        MakeOperator(TOKEN_TYPE::ASSIGN, TOKEN_TYPE::EQ);
        break;
      }
      default:
      {
        if (is_numeric(c))
        {
          MakeNumber();
          break;
        }
        if (is_letter(c) || c == '_')
        {
          MakeIdentifierOrKeyword();
        }
      }
    }
  }
  CheckInsertSemicolon();
  if (m_Tokens.empty() || m_Tokens[m_Tokens.size()-1].type != TOKEN_TYPE::TT_EOF)
    m_Tokens.emplace_back(TOKEN_TYPE::TT_EOF);
  return std::move(m_Tokens);
}

auto Lexer::CheckInsertSemicolon() -> void
{
  if (m_InsideExportList || m_InsideImportList) return;

  if (m_ParenDepth != 0 || m_SqBraceDepth != 0) return;

  if (m_Tokens.empty()) return;

  char next_non_white_space = '\0';
  for (auto i = m_Pos + 1; i < m_Text.size(); i++)
  {
    if (m_Text[i] == '/' && i + 1 < m_Text.size() && m_Text[i + 1] == '/')
    {
      while (i < m_Text.size() && m_Text[i] != '\n') i++;
    }

    next_non_white_space = m_Text[i];
    if (next_non_white_space != '\0' &&
        next_non_white_space != '\r' &&
        next_non_white_space != '\t' &&
        next_non_white_space != ' '  &&
        next_non_white_space != '\n')
    {
      break;
    }
  }

  const auto& last_token = m_Tokens.back();

  if (last_token.type == TOKEN_TYPE::PLUS ||
      last_token.type == TOKEN_TYPE::MINUS ||
      last_token.type == TOKEN_TYPE::TIMES ||
      last_token.type == TOKEN_TYPE::MOD ||
      last_token.type == TOKEN_TYPE::SLASH ||
      last_token.type == TOKEN_TYPE::PERIOD ||
      last_token.type == TOKEN_TYPE::NOT ||
      last_token.type == TOKEN_TYPE::LT ||
      last_token.type == TOKEN_TYPE::GT ||
      last_token.type == TOKEN_TYPE::LPAREN ||
      last_token.type == TOKEN_TYPE::LBRACK ||
      last_token.type == TOKEN_TYPE::LSQBRACK ||
      last_token.type == TOKEN_TYPE::SEMICOLON ||
      last_token.type == TOKEN_TYPE::COMMA)
  {
    return;
  }

  if (last_token.type == TOKEN_TYPE::RBRACK && !m_LastClosedBraceWasExpression)
  {
    return;
  }

  if (next_non_white_space == '}')
  {
    if (!m_BraceStack.empty() && m_BraceStack.back() == BraceKind::OBJECT)
    {
      return;
    }
  }

  if (next_non_white_space == '{' ||
      next_non_white_space == '.' ||
      next_non_white_space == '=')
  {
    return;
  }

  if (!m_BraceStack.empty()) {
    auto closed_kind = m_BraceStack.back();
    if (closed_kind == BraceKind::OBJECT) return;
  }

  m_Tokens.push_back(Token{
    .type = TOKEN_TYPE::SEMICOLON,
    .range = Range{
      .start = Position{.line = m_LinePos, .character = m_Pos + 1 },
      .end = Position{.line = m_LinePos, .character = m_Pos + 1 },
    },
  });
}

auto Lexer::MakeNumber() -> void
{
  const std::size_t textLength = m_Text.size();
  const std::size_t startColumn = m_ColPos;
  const std::size_t start = m_Pos;
  bool isFloat = false;

  while (m_Pos < textLength)
  {
    const char character = m_Text[m_Pos];

    if (is_numeric(character))
    {
      ++m_Pos;
      ++m_ColPos;
      continue;
    }

    if (character == '.')
    {
      if (isFloat) { break; }

      isFloat = true;
      ++m_Pos;
      ++m_ColPos;
      continue;
    }

    break;
  }

  const std::string value{m_Text.substr(start, m_Pos - start)};

  m_Tokens.push_back(Token{
    .type = isFloat ? TOKEN_TYPE::FLOAT : TOKEN_TYPE::INTEGER,
    .value = value,
    .range = Range{
      .start = Position{.line = m_LinePos, .character = startColumn},
      .end = Position{.line = m_LinePos, .character = m_ColPos}
    },
  });

  --m_Pos;
  --m_ColPos;
}

auto Lexer::MakeString() -> void
{
  const std::size_t textLength = m_Text.size();
  const std::size_t startColumn = m_ColPos;
  const std::size_t quotePosition = m_Pos;
  const char quote = m_Text[quotePosition];

  ++m_Pos;
  ++m_ColPos;

  std::string value;

  while (m_Pos < textLength)
  {
    const char character = m_Text[m_Pos];

    if (character == quote)
    {
      break;
    }

    if (character == '\\' && m_Pos + 1 < textLength)
    {
      switch (const char escapedCharacter = m_Text[m_Pos + 1])
      {
        case 'n':
          value.push_back('\n');
          m_Pos += 2;
          m_ColPos += 2;
          continue;

        case '\\':
          value.push_back('\\');
          m_Pos += 2;
          m_ColPos += 2;
          continue;

        default:
          value.push_back(character);
          ++m_Pos;
          ++m_ColPos;
          continue;
      }
    }

    value.push_back(character);
    ++m_Pos;
    ++m_ColPos;
  }

  m_Tokens.push_back(Token{
    .type = quote == '`' ? TOKEN_TYPE::FSTRING : TOKEN_TYPE::STRING,
    .value = std::move(value),
    .range = Range{
      .start = Position{ .line = m_LinePos, .character = startColumn },
      .end = Position{.line = m_LinePos, .character = m_ColPos }
    },
  });
}

auto Lexer::MakeFormatString() -> void // TODO: rewrite to handle {}
{
  MakeString();
}


const std::unordered_map<std::string_view, TOKEN_TYPE> KEYWORD_TABLE {
  { "or",     TOKEN_TYPE::OR },
  { "and",    TOKEN_TYPE::AND },
  { "if",     TOKEN_TYPE::IF },
  { "else",   TOKEN_TYPE::ELSE },
  { "for",    TOKEN_TYPE::FOR },
  { "while",  TOKEN_TYPE::WHILE },
  { "class",  TOKEN_TYPE::CLASS },
  { "fn",     TOKEN_TYPE::FN },
  { "let",    TOKEN_TYPE::LET },
  { "const",  TOKEN_TYPE::CONST },
  { "return", TOKEN_TYPE::RETURN },
  { "import", TOKEN_TYPE::IMPORT },
  { "export", TOKEN_TYPE::EXPORT },
  { "from",   TOKEN_TYPE::FROM },
};

auto Lexer::MakeIdentifierOrKeyword() -> void {
  const std::size_t text_len = m_Text.length();
  const std::size_t start = m_Pos;
  const std::size_t start_col_pos = m_ColPos;
  while (m_Pos != text_len)
  {
    m_Pos += 1;
    m_ColPos += 1;
    const char c = m_Text[m_Pos];
    if (!(is_letter(c) || is_numeric(c) || c == '_'))
    {
      m_Pos--;
      m_ColPos--;
      break;
    }
  }

  const auto tk = std::string_view{ m_Text.data() + start, m_Pos - start + 1 };

  const auto range = Range{
    .start = Position{ .line = m_LinePos, .character = start_col_pos },
    .end = Position{.line = m_LinePos, .character = m_ColPos }
  };

  if (const auto it = KEYWORD_TABLE.find(tk); it != KEYWORD_TABLE.end()) {
    if (it->second == TOKEN_TYPE::IMPORT) {
      m_PendingImport = true;
    } else if (it->second == TOKEN_TYPE::EXPORT) {
      m_PendingExport = true;
    } else if (m_PendingExport &&
               (it->second == TOKEN_TYPE::FN ||
                it->second == TOKEN_TYPE::LET ||
                it->second == TOKEN_TYPE::CONST ||
                it->second == TOKEN_TYPE::CLASS)) {
      m_PendingExport = false;
    }

    m_Tokens.push_back(Token{
      .type = it->second,
      .range = range
    });
    return;
  }

  m_Tokens.push_back(Token{
    .type = is_bool(tk) ? TOKEN_TYPE::BOOL : TOKEN_TYPE::IDENTIFIER,
    .value = std::string(tk),
    .range = range,
  });
}

auto Lexer::MakeOperator(TOKEN_TYPE regular, TOKEN_TYPE equal) -> void
{
  bool is_plusop = false;
  if (m_Pos + 1 < m_Text.length() && m_Text[m_Pos + 1] == '=')
  {
    is_plusop = true;
    m_Pos++;
    m_ColPos++;
  }

  m_Tokens.push_back(Token{
    .type = is_plusop ? equal : regular,
    .range = Range{
      .start = Position{.line = m_LinePos, .character = m_Pos },
      .end = Position{.line = m_LinePos, .character = m_Pos + is_plusop },
    },
  });
}


auto is_numeric(const char c) -> bool
{
  return c >= '0' && c <= '9';
}

auto is_letter(const char c) -> bool
{
  return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

auto is_bool(const std::string_view s) -> bool
{
  return s == "true" || s == "false";
}

}
