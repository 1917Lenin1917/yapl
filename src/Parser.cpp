//
// Created by lenin on 14.11.2024.
//

#include <memory>
#include <unordered_map>

#include "yapl/Parser.hpp"
#include "yapl/Token.hpp"
#include "yapl/ASTNode.hpp"
#include "yapl/exceptions/SyntaxError.hpp"

namespace yapl {

void Parser::advance(TOKEN_TYPE expected_token = TOKEN_TYPE::DEFAULT)
{
  m_pos++;
  check(expected_token);
}

void Parser::check(TOKEN_TYPE expected_token = TOKEN_TYPE::DEFAULT)
{
    if (expected_token == TOKEN_TYPE::DEFAULT)
        return;

    const auto& current_token = m_tokens[m_pos];
    if (current_token.type == expected_token)
        return;

    // If token is not what we expected, throw a SyntaxError exception
    // TODO: handle TT_EOF token better.
    throw SyntaxError(
            m_filename,
            current_token.line,
            current_token.col_start,
            current_token.col_end,
            m_source_lines[current_token.line-1],
            std::format("Expected token {} but got token {}", ttype_to_string(expected_token), ttype_to_string(current_token.type))
    );
}

std::unique_ptr<BaseASTNode> Parser::parse_literal()
{
  auto token = m_tokens[m_pos];
  auto location = location_from_token(token);
  advance();

  switch (token.type) {
    case TOKEN_TYPE::INTEGER: return std::make_unique<IntegerASTNode>(token, m_node_id++, location);
    case TOKEN_TYPE::STRING:
    case TOKEN_TYPE::FSTRING: return std::make_unique<StringASTNode>(token, m_node_id++, location);
    case TOKEN_TYPE::FLOAT: return std::make_unique<FloatASTNode>(token, m_node_id++, location);
    case TOKEN_TYPE::BOOL: return std::make_unique<BooleanASTNode>(token, m_node_id++, location);
    default:
      throw SyntaxError(
        m_filename,
        token.line,
        token.col_start,
        token.col_end,
        m_source_lines[token.line - 1],
        std::format("Unexpected token {} when parsing literal.", ttype_to_string(token.type))
      );
  }
}


std::unique_ptr<BaseASTNode> Parser::parse_starred_expr_or_expr()
{
  if (m_tokens[m_pos].type == TOKEN_TYPE::TIMES)
  {
    const auto star_token = m_tokens[m_pos];
    advance();

    auto expression = parse_expr();
    auto location = location_from_token_to_node(star_token, *expression);

    return std::make_unique<StarredExpressionASTNode>(
      std::move(expression),
      m_node_id++,
      location
    );
  }

  return parse_expr();
}

std::unique_ptr<BaseASTNode> Parser::parse_function_call(Token identifier)
{
  std::vector<std::unique_ptr<BaseASTNode>> args;
  auto start = token_start(identifier);

  advance();

  while (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
  {
    if (m_tokens[m_pos].type == TOKEN_TYPE::IDENTIFIER && m_tokens[m_pos + 1].type == TOKEN_TYPE::ASSIGN)
    {
      Token argument_name = m_tokens[m_pos];
      advance();
      advance();

      auto expression = parse_expr();
      auto location = SourceLocation{
        .start = token_start(argument_name),
        .end = expression->location.end
      };

      args.push_back(
        std::make_unique<KeyParamExpressionASTNode>(
          argument_name,
          std::move(expression),
          m_node_id++,
          location
        )
      );
    }
    else
    {
      args.push_back(parse_starred_expr_or_expr());
    }

    if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
      advance();

    if (m_tokens[m_pos].type == TOKEN_TYPE::TT_EOF)
    {
      throw SyntaxError(
        m_filename,
        identifier.line,
        identifier.col_start,
        identifier.col_end,
        m_source_lines[identifier.line - 1],
        std::format("Expected {} after function call.", ttype_to_string(TOKEN_TYPE::RPAREN))
      );
    }
  }

  auto closing_paren = m_tokens[m_pos];
  advance();

  auto identifier_node = std::make_unique<IdentifierASTNode>(
    identifier,
    m_node_id++,
    location_from_token(identifier)
  );

  auto call_location = SourceLocation{
    .start = start,
    .end = token_end(closing_paren)
  };

  return std::make_unique<FunctionCallASTNode>(
    std::move(identifier_node),
    args,
    m_node_id++,
    call_location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_method_or_property_call(Token identifier)
{
  advance();

  auto name = m_tokens[m_pos];
  advance();

  auto identifier_node = std::make_unique<IdentifierASTNode>(
    identifier,
    m_node_id++,
    location_from_token(identifier)
  );

  if (m_tokens[m_pos].type != TOKEN_TYPE::LPAREN)
  {
    auto location = SourceLocation{
      .start = identifier_node->location.start,
      .end = token_end(name)
    };

    return std::make_unique<GetPropertyASTNode>(
      std::move(identifier_node),
      name,
      m_node_id++,
      location
    );
  }

  std::vector<std::unique_ptr<BaseASTNode>> args;

  advance();

  while (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
  {
    args.push_back(parse_expr());
    if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
    {
      advance();
    }
  }

  auto closing_paren = m_tokens[m_pos];
  advance();

  auto location = SourceLocation{
    .start = identifier_node->location.start,
    .end = token_end(closing_paren)
  };

  return std::make_unique<MethodCallASTNode>(
    std::move(identifier_node),
    name,
    args,
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_indexing(Token identifier)
{
  auto identifier_node = std::make_unique<IdentifierASTNode>(
    identifier,
    m_node_id++,
    location_from_token(identifier)
  );

  advance();

  auto index_expr = parse_expr();

  check(TOKEN_TYPE::RSQBRACK);
  auto closing_bracket = m_tokens[m_pos];
  advance();

  auto location = SourceLocation{
    .start = identifier_node->location.start,
    .end = token_end(closing_bracket)
  };

  return std::make_unique<IndexASTNode>(
    std::move(identifier_node),
    std::move(index_expr),
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_identifier()
{
  auto identifier = m_tokens[m_pos];
  auto identifier_location = location_from_token(identifier);
  advance();

  if (m_tokens[m_pos].type == TOKEN_TYPE::LPAREN)
    return parse_function_call(identifier);

  if (m_tokens[m_pos].type == TOKEN_TYPE::LSQBRACK)
    return parse_indexing(identifier);

  if (m_tokens[m_pos].type == TOKEN_TYPE::PERIOD)
    return parse_property_or_method_chain(
      std::make_unique<IdentifierASTNode>(identifier, m_node_id++, identifier_location)
    );

  return std::make_unique<IdentifierASTNode>(identifier, m_node_id++, identifier_location);
}

std::unique_ptr<BaseASTNode> Parser::parse_array()
{
  auto open_bracket = m_tokens[m_pos];
  advance();

  std::vector<std::unique_ptr<BaseASTNode>> values;
  while (m_tokens[m_pos].type != TOKEN_TYPE::RSQBRACK)
  {
    auto expr = parse_expr();
    values.push_back(std::move(expr));

    if (m_tokens[m_pos].type == TOKEN_TYPE::RSQBRACK)
      break;

    check(TOKEN_TYPE::COMMA);
    advance();
  }

  auto close_bracket = m_tokens[m_pos];
  advance();

  auto location = location_from_tokens(open_bracket, close_bracket);
  return std::make_unique<ArrayASTNode>(values, m_node_id++, location);
}

std::unique_ptr<BaseASTNode> Parser::parse_dict()
{
  auto open_brace = m_tokens[m_pos];
  advance();

  std::vector<std::unique_ptr<BaseASTNode>> keys;
  std::vector<std::unique_ptr<BaseASTNode>> values;

  while (m_tokens[m_pos].type != TOKEN_TYPE::RBRACK)
  {
    auto key = parse_expr();

    check(TOKEN_TYPE::COLON);
    advance();

    auto value = parse_expr();

    keys.push_back(std::move(key));
    values.push_back(std::move(value));

    if (m_tokens[m_pos].type == TOKEN_TYPE::RBRACK)
      break;

    check(TOKEN_TYPE::COMMA);
    advance();
  }

  auto close_brace = m_tokens[m_pos];
  advance();

  auto location = location_from_tokens(open_brace, close_brace);
  return std::make_unique<DictASTNode>(std::move(keys), std::move(values), m_node_id++, location);
}

std::unique_ptr<BaseASTNode> Parser::parse_primary_expr()
{
	switch (m_tokens[m_pos].type)
	{
		default:
		{
            const auto& current_token = m_tokens[m_pos];
            throw SyntaxError(
                    m_filename,
                    current_token.line,
                    current_token.col_start,
                    current_token.col_end,
                    m_source_lines[current_token.line-1],
                    std::format("Unexpected token {} while parsing an expression.", ttype_to_string(current_token.type))
            );
		}
		case TOKEN_TYPE::INTEGER:
		case TOKEN_TYPE::FLOAT:
		case TOKEN_TYPE::STRING:
		case TOKEN_TYPE::FSTRING:
		case TOKEN_TYPE::BOOL: { return parse_literal(); }
		case TOKEN_TYPE::IDENTIFIER: { return parse_identifier(); }
		case TOKEN_TYPE::LPAREN: { return parse_paren_expr(); }
		case TOKEN_TYPE::LSQBRACK: { return parse_array(); }
		case TOKEN_TYPE::LBRACK: { return parse_dict(); }
		case TOKEN_TYPE::PLUS:
		case TOKEN_TYPE::MINUS:
		case TOKEN_TYPE::NOT: { return parse_unary(); }
	}
}

std::unique_ptr<BaseASTNode> Parser::parse_unary()
{
  auto op = m_tokens[m_pos];
  advance();

  std::unique_ptr<BaseASTNode> expr;
  if (m_tokens[m_pos].type == TOKEN_TYPE::LPAREN)
    expr = parse_paren_expr();
  else
    expr = parse_primary_expr();

  auto location = location_from_token_to_node(op, *expr);
  return std::make_unique<UnaryOpASTNode>(op, std::move(expr), m_node_id++, location);
}

std::unique_ptr<BaseASTNode> Parser::parse_paren_expr()
{
	auto open_paren = m_tokens[m_pos];
	advance();

	auto expr = parse_expr();
	if (expr == nullptr)
		return nullptr;

	check(TOKEN_TYPE::RPAREN);
	advance();

	if (m_tokens[m_pos].type == TOKEN_TYPE::LSQBRACK)
	{
		advance();

		auto index_expr = parse_expr();

		check(TOKEN_TYPE::RSQBRACK);
		auto closing_bracket = m_tokens[m_pos];
		advance();

		auto location = SourceLocation{
			.start = token_start(open_paren),
			.end = token_end(closing_bracket)
		};

		return std::make_unique<IndexASTNode>(
			std::move(expr),
			std::move(index_expr),
			m_node_id++,
			location
		);
	}

	if (m_tokens[m_pos].type == TOKEN_TYPE::PERIOD)
	{
		advance();

		auto name = m_tokens[m_pos];
		check(TOKEN_TYPE::IDENTIFIER);
		advance();

		if (m_tokens[m_pos].type != TOKEN_TYPE::LPAREN)
		{
			auto location = SourceLocation{
				.start = token_start(open_paren),
				.end = token_end(name)
			};

			return std::make_unique<GetPropertyASTNode>(
				std::move(expr),
				name,
				m_node_id++,
				location
			);
		}

		std::vector<std::unique_ptr<BaseASTNode>> args;

		check(TOKEN_TYPE::LPAREN);
		advance();

		while (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
		{
			args.push_back(parse_expr());
			if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
			{
				advance();
			}
		}

		auto closing_paren = m_tokens[m_pos];
		advance();

		auto location = SourceLocation{
			.start = token_start(open_paren),
			.end = token_end(closing_paren)
		};

		return std::make_unique<MethodCallASTNode>(
			std::move(expr),
			name,
			args,
			m_node_id++,
			location
		);
	}

	return expr;
}

std::unique_ptr<BaseASTNode> Parser::parse_expr()
{
	auto LHS = parse_primary_expr();

	if (LHS == nullptr)
		return nullptr;

	return std::move(parse_binop_rhs(0, std::move(LHS)));
}

std::unique_ptr<BaseASTNode> Parser::parse_semic_expr()
{
	auto expr = parse_expr();

	check(TOKEN_TYPE::SEMICOLON);
	advance(); //eat ;
	return expr;
}

std::unique_ptr<BaseASTNode> Parser::parse_binop_rhs(int expr_prec, std::unique_ptr<BaseASTNode> lhs)
{
  while (true)
  {
    auto token_prec = get_token_precedence(m_tokens[m_pos]);
    if (token_prec < expr_prec)
      return lhs;

    auto op = m_tokens[m_pos];
    advance();

    auto rhs = parse_primary_expr();
    if (!rhs)
      return nullptr;

    int next_prec = get_token_precedence(m_tokens[m_pos]);
    if (token_prec < next_prec)
    {
      rhs = parse_binop_rhs(token_prec + 1, std::move(rhs));
      if (!rhs)
        return nullptr;
    }

    auto location = SourceLocation{
      .start = lhs->location.start,
      .end = rhs->location.end
    };

    lhs = std::make_unique<BinaryOpASTNode>(op, std::move(lhs), std::move(rhs), m_node_id++, location);
  }
}

std::vector<std::unique_ptr<BaseASTNode>> Parser::parse_variable_declaration()
{
  std::vector<std::unique_ptr<BaseASTNode>> ret_val;
  auto decl_token = m_tokens[m_pos];
  advance();

  while (true)
  {
    if (m_tokens[m_pos].type != TOKEN_TYPE::IDENTIFIER)
    {
      throw SyntaxError(
        m_filename,
        decl_token.line,
        decl_token.col_start,
        m_tokens[m_pos].col_end,
        m_source_lines[decl_token.line - 1],
        std::format(
          "Expected identifier after variable declaration, but instead got {}",
          ttype_to_string(m_tokens[m_pos].type)
        )
      );
    }

    auto name_identifier = m_tokens[m_pos];
    advance();

    if (decl_token.type == TOKEN_TYPE::CONST)
      check(TOKEN_TYPE::ASSIGN);

    if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
    {
      advance();

      auto expr = parse_expr();
      auto location = SourceLocation{
        .start = token_start(decl_token),
        .end = expr->location.end
      };

      ret_val.push_back(
        std::make_unique<VariableASTNode>(
          decl_token,
          name_identifier,
          std::move(expr),
          m_node_id++,
          location
        )
      );

      if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
      {
        advance();
        continue;
      }

      if (m_tokens[m_pos].type == TOKEN_TYPE::SEMICOLON)
      {
        advance();
        return ret_val;
      }

      throw SyntaxError(
        m_filename,
        decl_token.line,
        decl_token.col_start,
        decl_token.col_start,
        m_source_lines[decl_token.line - 1],
        std::format("Expected semicolon after variable declaration")
      );
    }

    auto location = SourceLocation{
      .start = token_start(decl_token),
      .end = token_end(name_identifier)
    };

    ret_val.push_back(
      std::make_unique<VariableASTNode>(
        decl_token,
        name_identifier,
        nullptr,
        m_node_id++,
        location
      )
    );

    if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
    {
      advance();
      continue;
    }

    if (m_tokens[m_pos].type == TOKEN_TYPE::SEMICOLON)
    {
      advance();
      return ret_val;
    }

    throw SyntaxError(
      m_filename,
      decl_token.line,
      decl_token.col_start,
      name_identifier.col_end,
      m_source_lines[decl_token.line - 1],
      std::format("Expected semicolon after variable declaration")
    );
  }
}

std::unique_ptr<BaseASTNode> Parser::parse_method_call(std::unique_ptr<BaseASTNode> identifier)
{
  auto name = m_tokens[m_pos];
  advance();

  check(TOKEN_TYPE::LPAREN);
  advance();

  std::vector<std::unique_ptr<BaseASTNode>> args;
  while (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
  {
    args.push_back(parse_expr());
    if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
      advance();
  }

  auto closing_paren = m_tokens[m_pos];
  advance();

  auto location = SourceLocation{
    .start = identifier->location.start,
    .end = token_end(closing_paren)
  };

  return std::make_unique<MethodCallASTNode>(
    std::move(identifier),
    name,
    args,
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_property_get(std::unique_ptr<BaseASTNode> identifier)
{
  auto name = m_tokens[m_pos];
  advance();

  if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
  {
    advance();
    auto expr = parse_expr();

    auto location = SourceLocation{
      .start = identifier->location.start,
      .end = expr->location.end
    };

    return std::make_unique<SetPropertyASTNode>(
      std::move(identifier),
      name,
      std::move(expr),
      m_node_id++,
      location
    );
  }

  auto location = SourceLocation{
    .start = identifier->location.start,
    .end = token_end(name)
  };

  return std::make_unique<GetPropertyASTNode>(
    std::move(identifier),
    name,
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_property_or_method_chain(std::unique_ptr<BaseASTNode> identifier)
{
	if (m_tokens[m_pos].type != TOKEN_TYPE::PERIOD)
		return identifier;

	check(TOKEN_TYPE::PERIOD);
	advance();

	// Line.make().print()

	check(TOKEN_TYPE::IDENTIFIER); // make
	advance();
	if (m_tokens[m_pos].type == TOKEN_TYPE::LPAREN)
	{
		m_pos--;
		auto method = parse_method_call(std::move(identifier));
		return parse_property_or_method_chain(std::move(method));
	}
	m_pos--;
	auto property = parse_property_get(std::move(identifier));
	return parse_property_or_method_chain(std::move(property));
}

std::unique_ptr<BaseASTNode> Parser::parse_statement_or_ident()
{
  const auto &identifier = m_tokens[m_pos];
  auto identifier_location = location_from_token(identifier);
  advance();

  if (m_tokens[m_pos].type == TOKEN_TYPE::LPAREN)
  {
    auto expr = parse_function_call(identifier);
    check(TOKEN_TYPE::SEMICOLON);
    advance();
    return expr;
  }

  if (m_tokens[m_pos].type == TOKEN_TYPE::PERIOD)
  {
    auto chain = parse_property_or_method_chain(
      std::make_unique<IdentifierASTNode>(identifier, m_node_id++, identifier_location)
    );
    check(TOKEN_TYPE::SEMICOLON);
    advance();
    return chain;
  }

  if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
  {
    advance();

    auto expr = parse_semic_expr();

    auto lhs = std::make_unique<IdentifierASTNode>(identifier, m_node_id++, identifier_location);
    auto location = SourceLocation{
      .start = lhs->location.start,
      .end = expr->location.end
    };

    return std::make_unique<StatementASTNode>(
      std::move(lhs),
      std::move(expr),
      m_node_id++,
      location
    );
  }

  if (
    m_tokens[m_pos].type == TOKEN_TYPE::PLUSEQ ||
    m_tokens[m_pos].type == TOKEN_TYPE::MINUSEQ ||
    m_tokens[m_pos].type == TOKEN_TYPE::TIMESEQ ||
    m_tokens[m_pos].type == TOKEN_TYPE::MODEQ ||
    m_tokens[m_pos].type == TOKEN_TYPE::SLASHEQ
  )
  {
    static std::unordered_map<TOKEN_TYPE, TOKEN_TYPE> tt_to_tt = {
      {TOKEN_TYPE::PLUSEQ, TOKEN_TYPE::PLUS},
      {TOKEN_TYPE::MINUSEQ, TOKEN_TYPE::MINUS},
      {TOKEN_TYPE::TIMESEQ, TOKEN_TYPE::TIMES},
      {TOKEN_TYPE::MODEQ, TOKEN_TYPE::MOD},
      {TOKEN_TYPE::SLASHEQ, TOKEN_TYPE::SLASH},
    };

    const auto assignment_token = m_tokens[m_pos];
    auto new_token = tt_to_tt[assignment_token.type];
    advance();

    auto expr = parse_semic_expr();

    auto lhs_for_binary = std::make_unique<IdentifierASTNode>(
      identifier,
      m_node_id++,
      identifier_location
    );

    Token op_token{new_token};
    op_token.line = assignment_token.line;
    op_token.col_start = assignment_token.col_start;
    op_token.col_end = assignment_token.col_end;

    auto expanded_location = SourceLocation{
      .start = lhs_for_binary->location.start,
      .end = expr->location.end
    };

    auto expanded_expr = std::make_unique<BinaryOpASTNode>(
      op_token,
      std::move(lhs_for_binary),
      std::move(expr),
      m_node_id++,
      expanded_location
    );

    auto lhs_for_statement = std::make_unique<IdentifierASTNode>(
      identifier,
      m_node_id++,
      identifier_location
    );

    auto statement_location = SourceLocation{
      .start = lhs_for_statement->location.start,
      .end = expanded_expr->location.end
    };

    return std::make_unique<StatementASTNode>(
      std::move(lhs_for_statement),
      std::move(expanded_expr),
      m_node_id++,
      statement_location
    );
  }

  if (m_tokens[m_pos].type == TOKEN_TYPE::LSQBRACK)
  {
    m_pos--;
    auto index_expr = parse_identifier();

    if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
    {
      advance();

      auto expr = parse_semic_expr();
      auto location = SourceLocation{
        .start = index_expr->location.start,
        .end = expr->location.end
      };

      return std::make_unique<StatementIndexASTNode>(
        std::move(index_expr),
        std::move(expr),
        m_node_id++,
        location
      );
    }

    check(TOKEN_TYPE::SEMICOLON);
    advance();
    return index_expr;
  }

  if (m_tokens[m_pos].type == TOKEN_TYPE::SEMICOLON)
  {
    advance();
    return std::make_unique<IdentifierASTNode>(identifier, m_node_id++, identifier_location);
  }

  m_pos--;
  return parse_semic_expr();
}

std::unique_ptr<BaseASTNode> Parser::parse_import()
{
	const auto import_token = m_tokens[m_pos];
	check(TOKEN_TYPE::IMPORT);
	advance();

	check(TOKEN_TYPE::LBRACK);
	advance();

	std::vector<Token> identifiers;

	while (m_tokens[m_pos].type != TOKEN_TYPE::RBRACK)
	{
		check(TOKEN_TYPE::IDENTIFIER);
		auto id = m_tokens[m_pos];
		identifiers.push_back(id);
		advance();

		if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
		{
			advance();
		}
	}

	advance();

	check(TOKEN_TYPE::FROM);
	advance();

	check(TOKEN_TYPE::STRING);
	auto module = parse_literal();

	check(TOKEN_TYPE::SEMICOLON);
	advance();

	auto location = SourceLocation{
		.start = token_start(import_token),
		.end = module->location.end
	};

	return std::make_unique<ImportASTNode>(
		std::move(identifiers),
		std::move(module),
		m_node_id++,
		location
	);
}

std::unique_ptr<BaseASTNode> Parser::parse_export()
{
  const auto export_token = m_tokens[m_pos];
  check(TOKEN_TYPE::EXPORT);
  advance();

  check(TOKEN_TYPE::LBRACK);
  advance();

  std::vector<std::unique_ptr<BaseASTNode>> vars;

  while (m_tokens[m_pos].type != TOKEN_TYPE::RBRACK)
  {
    check(TOKEN_TYPE::IDENTIFIER);
    auto id = m_tokens[m_pos];
    advance();

    vars.push_back(
      std::make_unique<IdentifierASTNode>(
        id,
        m_node_id++,
        location_from_token(id)
      )
    );

    if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
    {
      advance();
    }
  }

  check(TOKEN_TYPE::RBRACK);
  auto closing_brace = m_tokens[m_pos];
  advance();

  auto location = location_from_tokens(export_token, closing_brace);

  return std::make_unique<ExportASTNode>(
    std::move(vars),
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_function_arguments()
{
  const auto open_paren = m_tokens[m_pos];
  check(TOKEN_TYPE::LPAREN);
  advance();

  std::vector<std::unique_ptr<FunctionArgumentASTNode>> args;
  std::unique_ptr<FunctionArgumentASTNode> args_arg;
  std::unique_ptr<FunctionArgumentASTNode> kwargs_arg;
  bool has_args = false;
  bool has_kwargs = false;

  while (m_pos < m_tokens.size())
  {
    if (m_tokens[m_pos].type == TOKEN_TYPE::RPAREN)
    {
      const auto closing_paren = m_tokens[m_pos];
      advance();

      auto location = location_from_tokens(open_paren, closing_paren);

      return std::make_unique<FunctionArgumentListASTNode>(
        args,
        std::move(args_arg),
        std::move(kwargs_arg),
        m_node_id++,
        location
      );
    }

    if (m_tokens[m_pos].type == TOKEN_TYPE::TIMES)
    {
      const auto first_star = m_tokens[m_pos];

      if (m_tokens[m_pos + 1].type == TOKEN_TYPE::TIMES)
      {
        if (has_kwargs)
        {
          throw SyntaxError(
            m_filename,
            m_tokens[m_pos + 1].line,
            m_tokens[m_pos].col_start,
            m_tokens[m_pos + 1].col_start,
            m_source_lines[m_tokens[m_pos + 1].line - 1],
            "Cannot have multiple **kwargs"
          );
        }

        advance();
        advance();

        check(TOKEN_TYPE::IDENTIFIER);
        Token identifier = m_tokens[m_pos];
        advance();

        has_kwargs = true;

        auto location = SourceLocation{
          .start = token_start(first_star),
          .end = token_end(identifier)
        };

        kwargs_arg = std::make_unique<FunctionArgumentASTNode>(
          identifier,
          Token{TOKEN_TYPE::IDENTIFIER, new char[]{"any"}},
          false,
          true,
          false,
          m_node_id++,
          location
        );

        if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
        {
          advance();
        }

        continue;
      }

      if (has_args)
      {
        throw SyntaxError(
          m_filename,
          m_tokens[m_pos].line,
          m_tokens[m_pos].col_start,
          m_tokens[m_pos].col_start,
          m_source_lines[m_tokens[m_pos].line - 1],
          "Cannot have multiple *args"
        );
      }

      advance();

      check(TOKEN_TYPE::IDENTIFIER);
      Token identifier = m_tokens[m_pos];
      advance();

      has_args = true;

      auto location = SourceLocation{
        .start = token_start(first_star),
        .end = token_end(identifier)
      };

      args_arg = std::make_unique<FunctionArgumentASTNode>(
        identifier,
        Token{TOKEN_TYPE::IDENTIFIER, new char[]{"any"}},
        true,
        false,
        false,
        m_node_id++,
        location
      );

      if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
      {
        advance();
      }

      continue;
    }

    if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
    {
      const auto assign_token = m_tokens[m_pos];
      advance();

      Token identifier = m_tokens[m_pos];
      check(TOKEN_TYPE::IDENTIFIER);
      advance();

      auto location = SourceLocation{
        .start = token_start(assign_token),
        .end = token_end(identifier)
      };

      args.push_back(
        std::make_unique<FunctionArgumentASTNode>(
          identifier,
          Token{TOKEN_TYPE::IDENTIFIER, new char[]{"any"}},
          false,
          false,
          true,
          m_node_id++,
          location
        )
      );

      if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
      {
        advance();
      }

      continue;
    }

    Token identifier = m_tokens[m_pos];
    check(TOKEN_TYPE::IDENTIFIER);
    advance();

    auto location = location_from_token(identifier);

    if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
    {
      advance();
    }

    args.push_back(
      std::make_unique<FunctionArgumentASTNode>(
        identifier,
        Token{TOKEN_TYPE::IDENTIFIER, new char[]{"any"}},
        false,
        false,
        false,
        m_node_id++,
        location
      )
    );
  }

  return nullptr;
}

std::unique_ptr<BaseASTNode> Parser::parse_function_declaration()
{
  const auto fn_token = m_tokens[m_pos];
  advance();

  auto fname = m_tokens[m_pos];
  check(TOKEN_TYPE::IDENTIFIER);
  advance();

  auto fargs = parse_function_arguments();

  auto location = SourceLocation{
    .start = token_start(fn_token),
    .end = fargs->location.end
  };

  return std::make_unique<FunctionDeclASTNode>(
    fname,
    std::move(fargs),
    Token{TOKEN_TYPE::IDENTIFIER, new char[]{"any"}},
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_return()
{
  auto return_token = m_tokens[m_pos];
  advance();

  auto expr = parse_semic_expr();
  auto location = location_from_token_to_node(return_token, *expr);

  return std::make_unique<ReturnStatementASTNode>(std::move(expr), m_node_id++, location);
}

std::unique_ptr<BaseASTNode> Parser::parse_ifelse_statement()
{
  const auto &if_token = m_tokens[m_pos];
  advance();

  auto condition = parse_expr();

  if (m_tokens[m_pos].type != TOKEN_TYPE::LBRACK)
  {
    const auto &current_token = m_tokens[m_pos];
    throw SyntaxError(
      m_filename,
      if_token.line,
      if_token.col_start,
      current_token.col_end,
      m_source_lines[if_token.line - 1],
      std::format(
        "Expected token {} after if-statement condition",
        ttype_to_string(TOKEN_TYPE::LBRACK)
      )
    );
  }

  auto true_scope = parse_scope();

  if (m_pos < m_tokens.size() && m_tokens[m_pos].type == TOKEN_TYPE::ELSE)
  {
    advance();

    if (m_pos < m_tokens.size() && m_tokens[m_pos].type == TOKEN_TYPE::IF)
    {
      auto false_scope = parse_ifelse_statement();

      auto location = SourceLocation{
        .start = token_start(if_token),
        .end = false_scope->location.end
      };

      return std::make_unique<IfElseExpressionASTNode>(
        std::move(condition),
        std::move(true_scope),
        std::move(false_scope),
        m_node_id++,
        location
      );
    }

    auto false_scope = parse_scope();

    auto location = SourceLocation{
      .start = token_start(if_token),
      .end = false_scope->location.end
    };

    return std::make_unique<IfElseExpressionASTNode>(
      std::move(condition),
      std::move(true_scope),
      std::move(false_scope),
      m_node_id++,
      location
    );
  }

  auto location = SourceLocation{
    .start = token_start(if_token),
    .end = true_scope->location.end
  };

  return std::make_unique<IfElseExpressionASTNode>(
    std::move(condition),
    std::move(true_scope),
    nullptr,
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_for_loop()
{
  const auto for_token = m_tokens[m_pos];
  advance();

  if (m_tokens[m_pos].type == TOKEN_TYPE::IDENTIFIER)
  {
    auto identifier = m_tokens[m_pos];
    advance();

    check(TOKEN_TYPE::COLON);
    advance();

    auto over = parse_expr();
    auto scope = parse_scope();

    auto location = SourceLocation{
      .start = token_start(for_token),
      .end = scope->location.end
    };

    return std::make_unique<ForEachLoopASTNode>(
      identifier,
      std::move(over),
      std::move(scope),
      m_node_id++,
      location
    );
  }

  auto declarations = parse_variable_declaration();
  auto declaration = std::move(declarations[0]);
  auto condition = parse_semic_expr();
  auto increment = parse_statement_or_ident();
  auto scope = parse_scope();

  auto location = SourceLocation{
    .start = token_start(for_token),
    .end = scope->location.end
  };

  return std::make_unique<ForLoopASTNode>(
    std::move(declaration),
    std::move(condition),
    std::move(increment),
    std::move(scope),
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_while_loop()
{
  const auto while_token = m_tokens[m_pos];
  advance();

  auto condition = parse_expr();
  auto scope = parse_scope();

  auto location = SourceLocation{
    .start = token_start(while_token),
    .end = scope->location.end
  };

  return std::make_unique<WhileLoopASTNode>(
    std::move(condition),
    std::move(scope),
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_class()
{
  const auto class_token = m_tokens[m_pos];
  advance();

  check(TOKEN_TYPE::IDENTIFIER);
  auto id = m_tokens[m_pos];
  advance();

  check(TOKEN_TYPE::LBRACK);
  advance();

  std::vector<std::unique_ptr<BaseASTNode>> fns;

  while (m_tokens[m_pos].type != TOKEN_TYPE::RBRACK)
  {
    fns.push_back(parse_function());
  }

  const auto closing_brace = m_tokens[m_pos];
  advance();

  auto location = location_from_tokens(class_token, closing_brace);

  return std::make_unique<ClassASTNode>(
    id,
    std::move(fns),
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_scope()
{
  check(TOKEN_TYPE::LBRACK);
  auto open_brace = m_tokens[m_pos];
  advance();

  auto scope = std::make_unique<ScopeASTNode>(
    m_node_id++,
    location_from_token(open_brace)
  );

  while (m_pos < m_tokens.size())
  {
    switch (m_tokens[m_pos].type)
    {
      case TOKEN_TYPE::RBRACK:
      {
        auto close_brace = m_tokens[m_pos];
        advance();
        scope->location = location_from_tokens(open_brace, close_brace);
        return scope;
      }

      case TOKEN_TYPE::RETURN:
      {
        scope->nodes.push_back(parse_return());
        break;
      }

      case TOKEN_TYPE::CONST:
      case TOKEN_TYPE::LET:
      {
        auto vars = parse_variable_declaration();
        for (auto &var : vars)
          scope->nodes.push_back(std::move(var));
        break;
      }

      case TOKEN_TYPE::IDENTIFIER:
      {
        scope->nodes.push_back(parse_statement_or_ident());
        break;
      }

      case TOKEN_TYPE::IF:
      {
        scope->nodes.push_back(parse_ifelse_statement());
        break;
      }

      case TOKEN_TYPE::WHILE:
      {
        scope->nodes.push_back(parse_while_loop());
        break;
      }

      case TOKEN_TYPE::FOR:
      {
        scope->nodes.push_back(parse_for_loop());
        break;
      }

      default:
      {
        scope->nodes.push_back(parse_semic_expr());
        break;
      }
    }
  }

  return scope;
}

std::unique_ptr<BaseASTNode> Parser::parse_function()
{
  auto decl = parse_function_declaration();
  auto body = parse_scope();

  auto location = SourceLocation{
    .start = decl->location.start,
    .end = body->location.end
  };

  return std::make_unique<FunctionASTNode>(
    std::move(decl),
    std::move(body),
    m_node_id++,
    location
  );
}

std::unique_ptr<BaseASTNode> Parser::parse_root()
{
  try
  {
    const auto start_token = m_tokens[m_pos];
    auto root = std::make_unique<RootASTNode>(
      m_node_id++,
      location_from_token(start_token)
    );

    while (m_pos < m_tokens.size())
    {
      switch (m_tokens[m_pos].type)
      {
        case TOKEN_TYPE::TT_EOF:
        {
          if (!root->nodes.empty())
          {
            root->location = SourceLocation{
              .start = token_start(start_token),
              .end = root->nodes.back()->location.end
            };
          }
          else
          {
            root->location = location_from_token(start_token);
          }

          return root;
        }

        case TOKEN_TYPE::CONST:
        case TOKEN_TYPE::LET:
        {
          auto vars = parse_variable_declaration();
          for (auto &var : vars)
            root->nodes.push_back(std::move(var));
          break;
        }

        case TOKEN_TYPE::IDENTIFIER:
        {
          root->nodes.push_back(parse_statement_or_ident());
          break;
        }

        case TOKEN_TYPE::FN:
        {
          root->nodes.push_back(parse_function());
          break;
        }

        case TOKEN_TYPE::IF:
        {
          root->nodes.push_back(parse_ifelse_statement());
          break;
        }

        case TOKEN_TYPE::WHILE:
        {
          root->nodes.push_back(parse_while_loop());
          break;
        }

        case TOKEN_TYPE::FOR:
        {
          root->nodes.push_back(parse_for_loop());
          break;
        }

        case TOKEN_TYPE::CLASS:
        {
          root->nodes.push_back(parse_class());
          break;
        }

        case TOKEN_TYPE::IMPORT:
        {
          root->nodes.push_back(parse_import());
          break;
        }

        case TOKEN_TYPE::EXPORT:
        {
          root->nodes.push_back(parse_export());
          break;
        }

        default:
        {
          root->nodes.push_back(parse_semic_expr());
          break;
        }
      }
    }

    if (!root->nodes.empty())
    {
      root->location = SourceLocation{
        .start = token_start(start_token),
        .end = root->nodes.back()->location.end
      };
    }
    else if (!m_tokens.empty())
    {
      root->location = location_from_token(start_token);
    }

    return root;
  }
  catch (const SyntaxError &error)
  {
    std::cerr << error.what() << std::endl;
    return nullptr;
  }
}

const Token &Parser::current_token() const
{
	return m_tokens[m_pos];
}

const Token &Parser::previous_token() const
{
	return m_tokens[m_pos - 1];
}

SourcePosition Parser::make_position(std::size_t line, std::size_t character) const
{
	return SourcePosition{
		.line = line,
		.character = character
	};
}

SourcePosition Parser::token_start(const Token &token) const
{
	return make_position(token.line, token.col_start);
}

SourcePosition Parser::token_end(const Token &token) const
{
	return make_position(token.line, token.col_end);
}

SourceLocation Parser::location_from_token(const Token &token) const
{
	return SourceLocation{
		.start = token_start(token),
		.end = token_end(token)
	};
}

SourceLocation Parser::location_from_tokens(const Token &start_token, const Token &end_token) const
{
	return SourceLocation{
		.start = token_start(start_token),
		.end = token_end(end_token)
	};
}

SourceLocation Parser::location_from_nodes(const BaseASTNode &start_node, const BaseASTNode &end_node) const
{
	return SourceLocation{
		.start = start_node.location.start,
		.end = end_node.location.end
	};
}

SourceLocation Parser::location_from_token_to_node(const Token &start_token, const BaseASTNode &end_node) const
{
	return SourceLocation{
		.start = token_start(start_token),
		.end = end_node.location.end
	};
}

SourceLocation Parser::location_from_node_to_token(const BaseASTNode &start_node, const Token &end_token) const
{
	return SourceLocation{
		.start = start_node.location.start,
		.end = token_end(end_token)
	};
}

}
