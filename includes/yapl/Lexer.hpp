//
// Created by lenin on 13.11.2024.
//

#pragma once
#include <cassert>
#include <string_view>
#include <vector>
#include <cstring>

#include "Token.hpp"

namespace yapl {

enum class BraceKind
{
    BLOCK,
    OBJECT,
    IMPORT_LIST,
    EXPORT_LIST,
};

class Lexer
{
public:
  explicit Lexer(const std::string_view text)
      : m_Text(text) {}

  auto Tokenize() -> std::vector<Token>;

private:
  auto CheckInsertSemicolon() -> void;
  auto MakeNumber() -> void;
  auto MakeString() -> void;
  auto MakeFormatString() -> void;
  auto MakeIdentifierOrKeyword() -> void;
  auto MakeOperator(TOKEN_TYPE regular, TOKEN_TYPE equal) -> void;

private:
  std::vector<Token> m_Tokens;

  std::string_view m_Text;
  std::size_t m_Pos = -1;
  std::size_t m_LinePos = 0;
  std::size_t m_ColPos = -1;
  std::size_t m_ParenDepth = 0;
  std::size_t m_BraceDepth = 0;
  std::size_t m_SqBraceDepth = 0;

  bool m_PendingImport = false;
  bool m_InsideImportList = false;

  bool m_PendingExport = false;
  bool m_InsideExportList = false;

  bool m_LastClosedBraceWasExpression = false;
  std::vector<BraceKind> m_BraceStack;

};

auto is_numeric(char c) -> bool;
auto is_letter(char c) -> bool;
auto is_bool(std::string_view s) -> bool;

}

