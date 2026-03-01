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
private:
  std::string m_filename;
  std::vector<std::string> m_source_lines;

  std::vector<Token> m_tokens;
  size_t m_pos;

  std::size_t m_node_id = 0;

private:
  const Token &current_token() const;
  const Token &previous_token() const;

  SourcePosition make_position(std::size_t line, std::size_t character) const;
  SourcePosition token_start(const Token &token) const;
  SourcePosition token_end(const Token &token) const;

  SourceLocation location_from_token(const Token &token) const;
  SourceLocation location_from_tokens(const Token &start_token, const Token &end_token) const;
  SourceLocation location_from_nodes(const BaseASTNode &start_node, const BaseASTNode &end_node) const;
  SourceLocation location_from_token_to_node(const Token &start_token, const BaseASTNode &end_node) const;
  SourceLocation location_from_node_to_token(const BaseASTNode &start_node, const Token &end_token) const;

public:
  explicit Parser(const std::vector<Token>& tokens, std::string filename, const std::vector<std::string>& source_lines)
    : m_filename(std::move(filename)), m_source_lines(source_lines), m_tokens(tokens), m_pos(0) {}

  void advance(TOKEN_TYPE expected_token);
  void check(TOKEN_TYPE expected_token);

  std::unique_ptr<BaseASTNode> parse_literal();
  std::unique_ptr<BaseASTNode> parse_function_call(Token identifier);
  std::unique_ptr<BaseASTNode> parse_method_or_property_call(Token identifier);
  std::unique_ptr<BaseASTNode> parse_indexing(Token identifier);
  std::unique_ptr<BaseASTNode> parse_identifier();
  std::unique_ptr<BaseASTNode> parse_array();
  std::unique_ptr<BaseASTNode> parse_dict();
  std::unique_ptr<BaseASTNode> parse_class();
  std::unique_ptr<BaseASTNode> parse_primary_expr();
  std::unique_ptr<BaseASTNode> parse_unary();
  std::unique_ptr<BaseASTNode> parse_paren_expr();
  std::unique_ptr<BaseASTNode> parse_expr();
  std::unique_ptr<BaseASTNode> parse_semic_expr();
  std::unique_ptr<BaseASTNode> parse_return();
  std::unique_ptr<BaseASTNode> parse_binop_rhs(int expr_prec, std::unique_ptr<BaseASTNode> lhs);
  std::vector<std::unique_ptr<BaseASTNode>> parse_variable_declaration();

  std::unique_ptr<BaseASTNode> parse_method_call(std::unique_ptr<BaseASTNode> identifier);
  std::unique_ptr<BaseASTNode> parse_property_get(std::unique_ptr<BaseASTNode> identifier);
  std::unique_ptr<BaseASTNode> parse_for_loop();
  std::unique_ptr<BaseASTNode> parse_while_loop();
  std::unique_ptr<BaseASTNode> parse_property_or_method_chain(std::unique_ptr<BaseASTNode> identifier);

  std::unique_ptr<BaseASTNode> parse_statement_or_ident();
  std::unique_ptr<BaseASTNode> parse_import();
  std::unique_ptr<BaseASTNode> parse_export();
  std::unique_ptr<BaseASTNode> parse_function_arguments();
  std::unique_ptr<BaseASTNode> parse_function_declaration();
  std::unique_ptr<BaseASTNode> parse_ifelse_statement();
  std::unique_ptr<BaseASTNode> parse_scope();
  std::unique_ptr<BaseASTNode> parse_starred_expr_or_expr();
  std::unique_ptr<BaseASTNode> parse_function();
  std::unique_ptr<BaseASTNode> parse_root();

  static int get_token_precedence(const Token& t)
  {
    switch (t.type)
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
};

}
