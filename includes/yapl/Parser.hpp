#pragma once
#include <iostream>
#include <memory>
#include <vector>

#include "Token.hpp"
#include "ASTNode.hpp"

using namespace yapl;

namespace yapl {

class BaseASTNode;

class Parser
{
public:
  explicit Parser(const std::vector<Token>& tokens, std::string filename, const std::vector<std::string>& source_lines)
    : m_Filename(std::move(filename)), m_SourceLines(source_lines), m_Tokens(tokens), m_Pos(0) {}

  auto Parse() -> ASTPtr;

private:
  std::string m_Filename;
  std::vector<std::string> m_SourceLines;

  std::vector<Token> m_Tokens;
  std::size_t m_Pos;

  std::size_t m_NodeId = 0;

private:
  [[nodiscard]] auto CurrentToken() const -> const Token&;
  [[nodiscard]] auto PreviousToken() const -> const Token&;
  [[nodiscard]] auto NextToken() const -> const Token&;

  auto Advance(TOKEN_TYPE expected_token) -> void;
  auto Check(TOKEN_TYPE expected_token) const -> void;


  [[nodiscard]] static Position MakePosition(std::size_t line, std::size_t character) ;
  [[nodiscard]] static Position TokenStart(const Token &token) ;
  [[nodiscard]] static Position TokenEnd(const Token &token) ;

  [[nodiscard]] static Range LocationFromToken(const Token &token) ;
  [[nodiscard]] static Range LocationFromTokens(const Token &start_token, const Token &end_token) ;
  [[nodiscard]] static Range LocationFromNodes(const BaseASTNode &start_node, const BaseASTNode &end_node);
  [[nodiscard]] static Range LocationFromTokenToNode(const Token &start_token, const BaseASTNode &end_node);
  [[nodiscard]] static Range LocationFromNodeToToken(const BaseASTNode &start_node, const Token &end_token);

  auto Literal() -> ASTPtr;
  auto FunctionCall(const Token& identifier) -> ASTPtr;
  auto PropertyGetOrMethodCall(const Token& identifier) -> ASTPtr;
  auto Indexing(const Token& identifier) -> ASTPtr;
  auto Identifier() -> ASTPtr;
  auto Array() -> ASTPtr;
  auto Dict() -> ASTPtr;
  auto Class() -> ASTPtr;
  auto PrimaryExpression() -> ASTPtr;
  auto Unary() -> ASTPtr;
  auto ParenExpression() -> ASTPtr;
  auto Expression() -> ASTPtr;
  auto SemicolonExpression() -> ASTPtr;
  auto Return() -> ASTPtr;
  auto BinOpRHS(int expr_prec, std::unique_ptr<BaseASTNode> lhs) -> ASTPtr;
  auto VariableDeclaration() -> std::vector<ASTPtr>;

  auto MethodCall(std::unique_ptr<BaseASTNode> identifier) -> ASTPtr;
  auto PropertyGet(std::unique_ptr<BaseASTNode> identifier) -> ASTPtr;
  auto ForLoop() -> ASTPtr;
  auto WhileLoop() -> ASTPtr;
  auto PropertyOrMethodChain(std::unique_ptr<BaseASTNode> identifier) -> ASTPtr;

  auto StatementOrIdentifier() -> ASTPtr;
  auto Import() -> ASTPtr;
  auto Export() -> std::vector<ASTPtr>;
  auto FunctionArguments() -> ASTPtr;
  auto FunctionDeclaration() -> ASTPtr;
  auto IfElseStatement() -> ASTPtr;
  auto Scope() -> ASTPtr;
  auto StarredExpressionOrExpression() -> ASTPtr;
  auto Function() -> ASTPtr;
};

static int get_token_precedence(const Token& token)
{
  switch (token.type)
  {
    case TOKEN_TYPE::NOT: { return 1000; }

    case TOKEN_TYPE::TIMES:
    case TOKEN_TYPE::SLASH:
    case TOKEN_TYPE::MOD: { return 900; }

    case TOKEN_TYPE::PLUS:
    case TOKEN_TYPE::MINUS: { return 800; }

    case TOKEN_TYPE::LT:
    case TOKEN_TYPE::LQ:
    case TOKEN_TYPE::GT:
    case TOKEN_TYPE::GQ: { return 700; }

    case TOKEN_TYPE::EQ:
    case TOKEN_TYPE::NEQ: { return 600; }

    case TOKEN_TYPE::AND: { return 500; }
    case TOKEN_TYPE::OR: { return 400; }

    default: { return -1; }
  }
}
}
