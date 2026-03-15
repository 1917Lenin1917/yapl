//
// Created by lenin on 14.11.2024.
//

#include <memory>
#include <unordered_map>
#include <vector>

#include "yapl/Parser.hpp"
#include "yapl/Token.hpp"
#include "yapl/ASTNode.hpp"
#include "yapl/exceptions/SyntaxError.hpp"

namespace yapl {

auto Parser::CurrentToken() const -> const Token&
{
  return m_Tokens[m_Pos];
}

auto Parser::PreviousToken() const -> const Token&
{
  return m_Tokens[m_Pos - 1];
}

auto Parser::NextToken() const -> const Token&
{
  return m_Tokens[m_Pos + 1];
}

auto Parser::Advance(const TOKEN_TYPE expected_token = TOKEN_TYPE::DEFAULT) -> void
{
  m_Pos++;
  Check(expected_token);
}

auto Parser::Check(const TOKEN_TYPE expected_token = TOKEN_TYPE::DEFAULT) const -> void
{
  if (expected_token == TOKEN_TYPE::DEFAULT) return;

  const auto& current_token = CurrentToken();
  if (current_token.type == expected_token) return;

  throw SyntaxError(
  m_Filename,
  current_token.range,
  m_SourceLines,
  std::format(
      "Expected token {} but got token {}",
      print_token_type(expected_token),
      print_token_type(current_token.type)
    )
  );
}

auto Parser::Literal() -> ASTPtr
{
  const auto& token = CurrentToken();
  auto location = LocationFromToken(token);
  Advance();

  switch (token.type) {
    case TOKEN_TYPE::INTEGER: return std::make_unique<IntegerASTNode>(token, m_NodeId++, location);
    case TOKEN_TYPE::STRING:
    case TOKEN_TYPE::FSTRING: return std::make_unique<StringASTNode>(token, m_NodeId++, location);
    case TOKEN_TYPE::FLOAT: return std::make_unique<FloatASTNode>(token, m_NodeId++, location);
    case TOKEN_TYPE::BOOL: return std::make_unique<BooleanASTNode>(token, m_NodeId++, location);
    default:
      throw SyntaxError(
        m_Filename,
        token.range,
        m_SourceLines,
        std::format(
          "Unexpected token {} when parsing literal.",
          print_token_type(token.type)
        )
      );
  }
}


auto Parser::StarredExpressionOrExpression() -> ASTPtr
{
  if (CurrentToken().type == TOKEN_TYPE::TIMES)
  {
    const auto star_token = CurrentToken();
    Advance();

    auto expression = Expression();
    auto location = LocationFromTokenToNode(star_token, *expression);

    return std::make_unique<StarredExpressionASTNode>(
      std::move(expression),
      m_NodeId++,
      location
    );
  }

  return Expression();
}

auto Parser::FunctionCall(const Token& identifier) -> ASTPtr
{
  std::vector<std::unique_ptr<BaseASTNode>> args;
  auto start = TokenStart(identifier);
  Advance();

  while (CurrentToken().type != TOKEN_TYPE::RPAREN)
  {
    if (CurrentToken().type == TOKEN_TYPE::IDENTIFIER && NextToken().type == TOKEN_TYPE::ASSIGN)
    {
      const auto& argument_name = CurrentToken();
      Advance();
      Advance();

      auto expression = Expression();
      auto location = Range{
        .start = TokenStart(argument_name),
        .end = expression->range.end
      };

      args.push_back(
        std::make_unique<KeyParamExpressionASTNode>(
          argument_name,
          std::move(expression),
          m_NodeId++,
          location
        )
      );
    }
    else
    {
      args.push_back(StarredExpressionOrExpression());
    }

    if (CurrentToken().type == TOKEN_TYPE::COMMA)
      Advance();

    if (CurrentToken().type == TOKEN_TYPE::TT_EOF)
    {
      throw SyntaxError(
        m_Filename,
        LocationFromTokenToNode(identifier, *args.back()),
        m_SourceLines,
        std::format("Expected {} after function call.", print_token_type(TOKEN_TYPE::RPAREN))
      );
    }
  }

  auto closing_paren = CurrentToken();
  Advance();

  auto identifier_node = std::make_unique<IdentifierASTNode>(
    identifier,
    m_NodeId++,
    LocationFromToken(identifier)
  );

  auto call_location = Range{
    .start = start,
    .end = TokenEnd(closing_paren)
  };

  return std::make_unique<FunctionCallASTNode>(
    std::move(identifier_node),
    args,
    m_NodeId++,
    call_location
  );
}

auto Parser::PropertyGetOrMethodCall(const Token& identifier) -> ASTPtr
{
  Advance();

  auto name = CurrentToken();
  Advance();

  auto identifier_node = std::make_unique<IdentifierASTNode>(
    identifier,
    m_NodeId++,
    LocationFromToken(identifier)
  );

  if (CurrentToken().type != TOKEN_TYPE::LPAREN)
  {
    auto location = Range{
      .start = identifier_node->range.start,
      .end = TokenEnd(name)
    };

    return std::make_unique<GetPropertyASTNode>(
      std::move(identifier_node),
      name,
      m_NodeId++,
      location
    );
  }

  std::vector<std::unique_ptr<BaseASTNode>> args;

  Advance();

  while (CurrentToken().type != TOKEN_TYPE::RPAREN)
  {
    args.push_back(Expression());
    if (CurrentToken().type == TOKEN_TYPE::COMMA)
    {
      Advance();
    }
  }

  auto closing_paren = CurrentToken();
  Advance();

  auto location = Range{
    .start = identifier_node->range.start,
    .end = TokenEnd(closing_paren)
  };

  return std::make_unique<MethodCallASTNode>(
    std::move(identifier_node),
    name,
    args,
    m_NodeId++,
    location
  );
}

auto Parser::Indexing(const Token& identifier) -> ASTPtr
{
  auto identifier_node = std::make_unique<IdentifierASTNode>(
    identifier,
    m_NodeId++,
    LocationFromToken(identifier)
  );

  Advance();

  auto index_expr = Expression();

  Check(TOKEN_TYPE::RSQBRACK);
  auto closing_bracket = CurrentToken();
  Advance();

  auto location = Range{
    .start = identifier_node->range.start,
    .end = TokenEnd(closing_bracket)
  };

  return std::make_unique<IndexASTNode>(
    std::move(identifier_node),
    std::move(index_expr),
    m_NodeId++,
    location
  );
}

auto Parser::Identifier() -> ASTPtr
{
  auto identifier = CurrentToken();
  auto identifier_location = LocationFromToken(identifier);
  Advance();

  switch (CurrentToken().type)
  {
    case TOKEN_TYPE::LPAREN: return FunctionCall(identifier);
    case TOKEN_TYPE::LSQBRACK: return Indexing(identifier);
    case TOKEN_TYPE::PERIOD: return PropertyOrMethodChain(std::make_unique<IdentifierASTNode>(identifier, m_NodeId++, identifier_location));

    default: return std::make_unique<IdentifierASTNode>(identifier, m_NodeId++, identifier_location);
  }
}

auto Parser::Array() -> ASTPtr
{
  auto open_bracket = CurrentToken();
  Advance();

  std::vector<std::unique_ptr<BaseASTNode>> values;
  while (CurrentToken().type != TOKEN_TYPE::RSQBRACK)
  {
    auto expr = Expression();
    values.push_back(std::move(expr));

    if (CurrentToken().type == TOKEN_TYPE::RSQBRACK)
      break;

    Check(TOKEN_TYPE::COMMA);
    Advance();
  }

  auto close_bracket = CurrentToken();
  Advance();

  auto location = LocationFromTokens(open_bracket, close_bracket);
  return std::make_unique<ArrayASTNode>(values, m_NodeId++, location);
}

auto Parser::Dict() -> ASTPtr
{
  const auto& open_brace = CurrentToken();
  Advance();

  std::vector<std::unique_ptr<BaseASTNode>> keys;
  std::vector<std::unique_ptr<BaseASTNode>> values;

  while (CurrentToken().type != TOKEN_TYPE::RBRACK)
  {
    auto key = Expression();

    Check(TOKEN_TYPE::COLON);
    Advance();

    auto value = Expression();

    keys.push_back(std::move(key));
    values.push_back(std::move(value));

    if (CurrentToken().type == TOKEN_TYPE::RBRACK)
      break;

    Check(TOKEN_TYPE::COMMA);
    Advance();
  }

  const auto& close_brace = CurrentToken();
  Advance();

  auto location = LocationFromTokens(open_brace, close_brace);
  return std::make_unique<DictASTNode>(std::move(keys), std::move(values), m_NodeId++, location);
}

auto Parser::PrimaryExpression() -> ASTPtr
{
	switch (CurrentToken().type)
	{
		default:
		{
      const auto& current_token = CurrentToken();
      throw SyntaxError(
        m_Filename,
        current_token.range,
        m_SourceLines,
        std::format("Unexpected token {} while parsing an expression.", print_token_type(current_token.type))
      );
		}
		case TOKEN_TYPE::INTEGER:
		case TOKEN_TYPE::FLOAT:
		case TOKEN_TYPE::STRING:
		case TOKEN_TYPE::FSTRING:
		case TOKEN_TYPE::BOOL:       { return Literal(); }
		case TOKEN_TYPE::IDENTIFIER: { return Identifier(); }
		case TOKEN_TYPE::LPAREN:     { return ParenExpression(); }
		case TOKEN_TYPE::LSQBRACK:   { return Array(); }
		case TOKEN_TYPE::LBRACK:     { return Dict(); }
		case TOKEN_TYPE::PLUS:
		case TOKEN_TYPE::MINUS:
		case TOKEN_TYPE::NOT:        { return Unary(); }
	}
}

auto Parser::Unary() -> ASTPtr
{
  const auto& op = CurrentToken();
  Advance();

  std::unique_ptr<BaseASTNode> expr = CurrentToken().type == TOKEN_TYPE::LPAREN ? ParenExpression() : PrimaryExpression();

  auto location = LocationFromTokenToNode(op, *expr);
  return std::make_unique<UnaryOpASTNode>(op, std::move(expr), m_NodeId++, location);
}

auto Parser::ParenExpression() -> ASTPtr
{
	const auto& open_paren = CurrentToken();
	Advance();

	auto expr = Expression();
	if (expr == nullptr)
		return nullptr;

	Check(TOKEN_TYPE::RPAREN);
	Advance();

	if (CurrentToken().type == TOKEN_TYPE::LSQBRACK)
	{
		Advance();

		auto index_expr = Expression();

		Check(TOKEN_TYPE::RSQBRACK);
		const auto& closing_bracket = CurrentToken();
		Advance();

		auto location = Range{
			.start = TokenStart(open_paren),
			.end = TokenEnd(closing_bracket)
		};

		return std::make_unique<IndexASTNode>(
			std::move(expr),
			std::move(index_expr),
			m_NodeId++,
			location
		);
	}

	if (CurrentToken().type == TOKEN_TYPE::PERIOD)
	{
		Advance();

		const auto& name = CurrentToken();
		Check(TOKEN_TYPE::IDENTIFIER);
		Advance();

		if (CurrentToken().type != TOKEN_TYPE::LPAREN)
		{
			auto location = Range{
				.start = TokenStart(open_paren),
				.end = TokenEnd(name)
			};

			return std::make_unique<GetPropertyASTNode>(
				std::move(expr),
				name,
				m_NodeId++,
				location
			);
		}

		std::vector<std::unique_ptr<BaseASTNode>> args;

		Check(TOKEN_TYPE::LPAREN);
		Advance();

		while (CurrentToken().type != TOKEN_TYPE::RPAREN)
		{
			args.push_back(Expression());
			if (CurrentToken().type == TOKEN_TYPE::COMMA)
			{
				Advance();
			}
		}

		const auto& closing_paren = CurrentToken();
		Advance();

		auto location = Range{
			.start = TokenStart(open_paren),
			.end = TokenEnd(closing_paren)
		};

		return std::make_unique<MethodCallASTNode>(
			std::move(expr),
			name,
			args,
			m_NodeId++,
			location
		);
	}

	return expr;
}

auto Parser::Expression() -> ASTPtr
{
	auto LHS = PrimaryExpression();

	if (LHS == nullptr)
		return nullptr;

	return std::move(BinOpRHS(0, std::move(LHS)));
}

auto Parser::SemicolonExpression() -> ASTPtr
{
	auto expr = Expression();

	Check(TOKEN_TYPE::SEMICOLON);
	Advance(); //eat ;

	return expr;
}

auto Parser::BinOpRHS(int expr_prec, std::unique_ptr<BaseASTNode> lhs) -> ASTPtr
{
  while (true)
  {
    auto token_prec = get_token_precedence(CurrentToken());
    if (token_prec < expr_prec) return lhs;

    const auto& op = CurrentToken();
    Advance();

    auto rhs = PrimaryExpression();
    if (!rhs) return nullptr;

    int next_prec = get_token_precedence(CurrentToken());
    if (token_prec < next_prec)
    {
      rhs = BinOpRHS(token_prec + 1, std::move(rhs));
      if (!rhs) return nullptr;
    }

    auto location = Range{
      .start = lhs->range.start,
      .end = rhs->range.end
    };

    lhs = std::make_unique<BinaryOpASTNode>(op, std::move(lhs), std::move(rhs), m_NodeId++, location);
  }
}

auto Parser::VariableDeclaration() -> std::vector<ASTPtr>
{
  std::vector<std::unique_ptr<BaseASTNode>> ret_val;
  auto decl_token = CurrentToken();
  Advance();

  while (true)
  {
    const auto& current_token = CurrentToken();
    if (current_token.type != TOKEN_TYPE::IDENTIFIER)
    {
      throw SyntaxError(
        m_Filename,
        LocationFromTokens(decl_token, current_token),
        m_SourceLines,
        std::format(
          "Expected identifier after variable declaration, but instead got {}",
          print_token_type(current_token.type)
        )
      );
    }

    const auto& name_identifier = CurrentToken();
    Advance();

    if (decl_token.type == TOKEN_TYPE::CONST)
      Check(TOKEN_TYPE::ASSIGN);

    if (CurrentToken().type == TOKEN_TYPE::ASSIGN)
    {
      Advance();

      auto expr = Expression();
      auto location = Range{
        .start = TokenStart(decl_token),
        .end = expr->range.end
      };

      ret_val.push_back(
        std::make_unique<VariableASTNode>(
          decl_token,
          name_identifier,
          std::move(expr),
          m_NodeId++,
          location
        )
      );

      if (CurrentToken().type == TOKEN_TYPE::COMMA)
      {
        Advance();
        continue;
      }

      if (CurrentToken().type == TOKEN_TYPE::SEMICOLON)
      {
        Advance();
        return ret_val;
      }

      throw SyntaxError(
        m_Filename,
        decl_token.range,
        m_SourceLines,
        std::format("Expected semicolon after variable declaration")
      );
    }

    auto location = Range{
      .start = TokenStart(decl_token),
      .end = TokenEnd(name_identifier)
    };

    ret_val.push_back(
      std::make_unique<VariableASTNode>(
        decl_token,
        name_identifier,
        nullptr,
        m_NodeId++,
        location
      )
    );

    if (CurrentToken().type == TOKEN_TYPE::COMMA)
    {
      Advance();
      continue;
    }

    if (CurrentToken().type == TOKEN_TYPE::SEMICOLON)
    {
      Advance();
      return ret_val;
    }

    throw SyntaxError(
      m_Filename,
      LocationFromTokens(decl_token, name_identifier),
      m_SourceLines,
      std::format("Expected semicolon after variable declaration")
    );
  }
}

auto Parser::MethodCall(std::unique_ptr<BaseASTNode> identifier) -> ASTPtr
{
  auto name = CurrentToken();
  Advance();

  Check(TOKEN_TYPE::LPAREN);
  Advance();

  std::vector<std::unique_ptr<BaseASTNode>> args;
  while (CurrentToken().type != TOKEN_TYPE::RPAREN)
  {
    args.push_back(Expression());
    if (CurrentToken().type == TOKEN_TYPE::COMMA)
      Advance();
  }

  auto closing_paren = CurrentToken();
  Advance();

  auto location = Range{
    .start = identifier->range.start,
    .end = TokenEnd(closing_paren)
  };

  return std::make_unique<MethodCallASTNode>(
    std::move(identifier),
    name,
    args,
    m_NodeId++,
    location
  );
}

auto Parser::PropertyGet(std::unique_ptr<BaseASTNode> identifier) -> ASTPtr
{
  auto name = CurrentToken();
  Advance();

  if (CurrentToken().type == TOKEN_TYPE::ASSIGN)
  {
    Advance();
    auto expr = Expression();

    auto location = Range{
      .start = identifier->range.start,
      .end = expr->range.end
    };

    return std::make_unique<SetPropertyASTNode>(
      std::move(identifier),
      name,
      std::move(expr),
      m_NodeId++,
      location
    );
  }

  auto location = Range{
    .start = identifier->range.start,
    .end = TokenEnd(name)
  };

  return std::make_unique<GetPropertyASTNode>(
    std::move(identifier),
    name,
    m_NodeId++,
    location
  );
}

auto Parser::PropertyOrMethodChain(std::unique_ptr<BaseASTNode> identifier) -> ASTPtr
{
	if (CurrentToken().type != TOKEN_TYPE::PERIOD)
		return identifier;

	Check(TOKEN_TYPE::PERIOD);
	Advance();

	// Line.make().print()

	Check(TOKEN_TYPE::IDENTIFIER); // make
	Advance();
	if (CurrentToken().type == TOKEN_TYPE::LPAREN)
	{
		m_Pos--;
		auto method = MethodCall(std::move(identifier));
		return PropertyOrMethodChain(std::move(method));
	}
	m_Pos--;
	auto property = PropertyGet(std::move(identifier));
	return PropertyOrMethodChain(std::move(property));
}

auto Parser::StatementOrIdentifier() -> ASTPtr
{
  const auto &identifier = CurrentToken();
  auto identifier_location = LocationFromToken(identifier);
  Advance();

  if (CurrentToken().type == TOKEN_TYPE::LPAREN)
  {
    auto expr = FunctionCall(identifier);
    Check(TOKEN_TYPE::SEMICOLON);
    Advance();
    return expr;
  }

  if (CurrentToken().type == TOKEN_TYPE::PERIOD)
  {
    auto chain = PropertyOrMethodChain(
      std::make_unique<IdentifierASTNode>(identifier, m_NodeId++, identifier_location)
    );
    Check(TOKEN_TYPE::SEMICOLON);
    Advance();
    return chain;
  }

  if (CurrentToken().type == TOKEN_TYPE::ASSIGN)
  {
    Advance();

    auto expr = SemicolonExpression();

    auto lhs = std::make_unique<IdentifierASTNode>(identifier, m_NodeId++, identifier_location);
    auto location = Range{
      .start = lhs->range.start,
      .end = expr->range.end
    };

    return std::make_unique<StatementASTNode>(
      std::move(lhs),
      std::move(expr),
      m_NodeId++,
      location
    );
  }

  const auto& current_token = CurrentToken();
  if (
    current_token.type == TOKEN_TYPE::PLUSEQ ||
    current_token.type == TOKEN_TYPE::MINUSEQ ||
    current_token.type == TOKEN_TYPE::TIMESEQ ||
    current_token.type == TOKEN_TYPE::MODEQ ||
    current_token.type == TOKEN_TYPE::SLASHEQ
  )
  {
    static std::unordered_map<TOKEN_TYPE, TOKEN_TYPE> tt_to_tt = {
      {TOKEN_TYPE::PLUSEQ, TOKEN_TYPE::PLUS},
      {TOKEN_TYPE::MINUSEQ, TOKEN_TYPE::MINUS},
      {TOKEN_TYPE::TIMESEQ, TOKEN_TYPE::TIMES},
      {TOKEN_TYPE::MODEQ, TOKEN_TYPE::MOD},
      {TOKEN_TYPE::SLASHEQ, TOKEN_TYPE::SLASH},
    };

    const auto assignment_token = CurrentToken();
    const auto& new_token = tt_to_tt[assignment_token.type];
    Advance();

    auto expr = SemicolonExpression();

    auto lhs_for_binary = std::make_unique<IdentifierASTNode>(
      identifier,
      m_NodeId++,
      identifier_location
    );

    Token op_token{.type = new_token, .range = assignment_token.range };

    auto expanded_location = Range{
      .start = lhs_for_binary->range.start,
      .end = expr->range.end
    };

    auto expanded_expr = std::make_unique<BinaryOpASTNode>(
      op_token,
      std::move(lhs_for_binary),
      std::move(expr),
      m_NodeId++,
      expanded_location
    );

    auto lhs_for_statement = std::make_unique<IdentifierASTNode>(
      identifier,
      m_NodeId++,
      identifier_location
    );

    auto statement_location = Range{
      .start = lhs_for_statement->range.start,
      .end = expanded_expr->range.end
    };

    return std::make_unique<StatementASTNode>(
      std::move(lhs_for_statement),
      std::move(expanded_expr),
      m_NodeId++,
      statement_location
    );
  }

  if (CurrentToken().type == TOKEN_TYPE::LSQBRACK)
  {
    m_Pos--;
    auto index_expr = Identifier();

    if (CurrentToken().type == TOKEN_TYPE::ASSIGN)
    {
      Advance();

      auto expr = SemicolonExpression();
      auto location = Range{
        .start = index_expr->range.start,
        .end = expr->range.end
      };

      return std::make_unique<StatementIndexASTNode>(
        std::move(index_expr),
        std::move(expr),
        m_NodeId++,
        location
      );
    }

    Check(TOKEN_TYPE::SEMICOLON);
    Advance();
    return index_expr;
  }

  if (CurrentToken().type == TOKEN_TYPE::SEMICOLON)
  {
    Advance();
    return std::make_unique<IdentifierASTNode>(identifier, m_NodeId++, identifier_location);
  }

  m_Pos--;
  return SemicolonExpression();
}

auto Parser::Import() -> ASTPtr
{
	const auto import_token = CurrentToken();
	Check(TOKEN_TYPE::IMPORT);
	Advance();

	Check(TOKEN_TYPE::LBRACK);
	Advance();

	std::vector<Token> identifiers;

	while (CurrentToken().type != TOKEN_TYPE::RBRACK)
	{
		Check(TOKEN_TYPE::IDENTIFIER);
		const auto& id = CurrentToken();
		identifiers.push_back(id);
		Advance();

		if (CurrentToken().type == TOKEN_TYPE::COMMA)
		{
			Advance();
		}
	}

	Advance();

	Check(TOKEN_TYPE::FROM);
	Advance();

	Check(TOKEN_TYPE::STRING);
	auto module = Literal();

	Check(TOKEN_TYPE::SEMICOLON);
	Advance();

	auto location = Range{
		.start = TokenStart(import_token),
		.end = module->range.end
	};

	return std::make_unique<ImportASTNode>(
		std::move(identifiers),
		std::move(module),
		m_NodeId++,
		location
	);
}

auto Parser::Export() -> std::vector<ASTPtr>
{
  const auto export_token = CurrentToken();
  Check(TOKEN_TYPE::EXPORT);
  Advance();

  std::vector<std::unique_ptr<BaseASTNode>> vars;
  std::vector<std::unique_ptr<BaseASTNode>> ret;

  const auto& next_token = CurrentToken();
  if (next_token.type == TOKEN_TYPE::FN)
  {
    auto fn = Function();
    auto name = static_cast<FunctionDeclASTNode*>(static_cast<FunctionASTNode*>(fn.get())->decl.get())->name;
    vars.push_back(
      std::make_unique<IdentifierASTNode>(
      name,
      m_NodeId++,
      LocationFromToken(name))
    );

    ret.push_back(std::move(fn));
    ret.push_back(std::make_unique<ExportASTNode>(std::move(vars), m_NodeId++, LocationFromToken(export_token)));

    return ret;
  }

  if (next_token.type == TOKEN_TYPE::CONST || next_token.type == TOKEN_TYPE::LET)
  {
    auto _vars = VariableDeclaration();
    for (auto& var : _vars)
    {
      auto name = static_cast<VariableASTNode*>(var.get())->name;
      vars.push_back(
        std::make_unique<IdentifierASTNode>(
        name,
        m_NodeId++,
        LocationFromToken(name)
        )
      );
      ret.push_back(std::move(var));
    }
    ret.push_back(std::make_unique<ExportASTNode>(std::move(vars), m_NodeId++, LocationFromToken(export_token)));

    return ret;
  }

  if (next_token.type == TOKEN_TYPE::CLASS)
  {
    auto fn = Class();
    auto name = static_cast<ClassASTNode*>(fn.get())->name;
    vars.push_back(
      std::make_unique<IdentifierASTNode>(
      name,
      m_NodeId++,
      LocationFromToken(name))
    );

    ret.push_back(std::move(fn));
    ret.push_back(std::make_unique<ExportASTNode>(std::move(vars), m_NodeId++, LocationFromToken(export_token)));

    return ret;
  }

  Check(TOKEN_TYPE::LBRACK);
  Advance();

  while (CurrentToken().type != TOKEN_TYPE::RBRACK)
  {
    Check(TOKEN_TYPE::IDENTIFIER);
    auto id = CurrentToken();
    Advance();

    vars.push_back(
      std::make_unique<IdentifierASTNode>(
        id,
        m_NodeId++,
        LocationFromToken(id)
      )
    );

    if (CurrentToken().type == TOKEN_TYPE::COMMA)
    {
      Advance();
    }
  }

  Check(TOKEN_TYPE::RBRACK);
  auto closing_brace = CurrentToken();
  Advance();

  auto location = LocationFromTokens(export_token, closing_brace);

  ret.push_back(std::move(std::make_unique<ExportASTNode>(
    std::move(vars),
    m_NodeId++,
    location
  )));

  return ret;
}

auto Parser::FunctionArguments() -> ASTPtr
{
  const auto& open_paren = CurrentToken();
  Check(TOKEN_TYPE::LPAREN);
  Advance();

  std::vector<std::unique_ptr<FunctionArgumentASTNode>> args;
  std::unique_ptr<FunctionArgumentASTNode> args_arg;
  std::unique_ptr<FunctionArgumentASTNode> kwargs_arg;
  bool has_args = false;
  bool has_kwargs = false;

  while (m_Pos < m_Tokens.size())
  {
    const auto& current_token = CurrentToken();
    if (current_token.type == TOKEN_TYPE::RPAREN)
    {
      Advance();

      auto location = LocationFromTokens(open_paren, current_token);

      return std::make_unique<FunctionArgumentListASTNode>(
        args,
        std::move(args_arg),
        std::move(kwargs_arg),
        m_NodeId++,
        location
      );
    }

    if (current_token.type == TOKEN_TYPE::TIMES)
    {
      if (NextToken().type == TOKEN_TYPE::TIMES)
      {
        if (has_kwargs)
        {
          throw SyntaxError(
            m_Filename,
            LocationFromNodeToToken(*kwargs_arg, NextToken()),
            m_SourceLines,
            "Cannot have multiple **kwargs"
          );
        }

        Advance();
        Advance();

        Check(TOKEN_TYPE::IDENTIFIER);
        const auto& identifier = CurrentToken();
        Advance();

        has_kwargs = true;

        auto location = Range{
          .start = TokenStart(current_token),
          .end = TokenEnd(identifier)
        };

        kwargs_arg = std::make_unique<FunctionArgumentASTNode>(
          identifier,
          Token{ .type = TOKEN_TYPE::IDENTIFIER, .value = "any" },
          false,
          true,
          false,
          m_NodeId++,
          location
        );

        if (CurrentToken().type == TOKEN_TYPE::COMMA)
        {
          Advance();
        }

        continue;
      }

      if (has_args)
      {
        throw SyntaxError(
          m_Filename,
          LocationFromNodeToToken(*args_arg, NextToken()),
          m_SourceLines,
          "Cannot have multiple *args"
        );
      }

      Advance();

      Check(TOKEN_TYPE::IDENTIFIER);
      const auto& identifier = CurrentToken();
      Advance();

      has_args = true;

      auto location = Range{
        .start = TokenStart(current_token),
        .end = TokenEnd(identifier)
      };

      args_arg = std::make_unique<FunctionArgumentASTNode>(
        identifier,
        Token{TOKEN_TYPE::IDENTIFIER, new char[]{"any"}},
        true,
        false,
        false,
        m_NodeId++,
        location
      );

      if (CurrentToken().type == TOKEN_TYPE::COMMA)
      {
        Advance();
      }

      continue;
    }

    if (CurrentToken().type == TOKEN_TYPE::ASSIGN)
    {
      const auto& assign_token = CurrentToken();
      Advance();

      const auto& identifier = CurrentToken();
      Check(TOKEN_TYPE::IDENTIFIER);
      Advance();

      auto location = Range{
        .start = TokenStart(assign_token),
        .end = TokenEnd(identifier)
      };

      args.push_back(
        std::make_unique<FunctionArgumentASTNode>(
          identifier,
          Token{ .type = TOKEN_TYPE::IDENTIFIER, .value = "any"},
          false,
          false,
          true,
          m_NodeId++,
          location
        )
      );

      if (CurrentToken().type == TOKEN_TYPE::COMMA)
      {
        Advance();
      }

      continue;
    }

    const auto& identifier = CurrentToken();
    Check(TOKEN_TYPE::IDENTIFIER);
    Advance();

    auto location = LocationFromToken(identifier);

    if (CurrentToken().type == TOKEN_TYPE::COMMA)
    {
      Advance();
    }

    args.push_back(
      std::make_unique<FunctionArgumentASTNode>(
        identifier,
        Token{TOKEN_TYPE::IDENTIFIER, new char[]{"any"}},
        false,
        false,
        false,
        m_NodeId++,
        location
      )
    );
  }

  return nullptr;
}

auto Parser::FunctionDeclaration() -> ASTPtr
{
  const auto& fn_token = CurrentToken();
  Advance();

  const auto& fname = CurrentToken();
  Check(TOKEN_TYPE::IDENTIFIER);
  Advance();

  auto fargs = FunctionArguments();

  auto location = Range{
    .start = TokenStart(fn_token),
    .end = fargs->range.end
  };

  return std::make_unique<FunctionDeclASTNode>(
    fname,
    std::move(fargs),
    Token{ .type = TOKEN_TYPE::IDENTIFIER, .value = "any" },
    m_NodeId++,
    location
  );
}

auto Parser::Return() -> ASTPtr
{
  const auto& return_token = CurrentToken();
  Advance();

  auto expr = SemicolonExpression();
  auto location = LocationFromTokenToNode(return_token, *expr);

  return std::make_unique<ReturnStatementASTNode>(std::move(expr), m_NodeId++, location);
}

auto Parser::IfElseStatement() -> ASTPtr
{
  const auto &if_token = CurrentToken();
  Advance();

  auto condition = Expression();

  if (CurrentToken().type != TOKEN_TYPE::LBRACK)
  {
    const auto &current_token = CurrentToken();
    throw SyntaxError(
      m_Filename,
      LocationFromTokens(if_token, current_token),
      m_SourceLines,
      std::format(
        "Expected token {} after if-statement condition",
        print_token_type(TOKEN_TYPE::LBRACK)
      )
    );
  }

  auto true_scope = Scope();

  if (m_Pos < m_Tokens.size() && CurrentToken().type == TOKEN_TYPE::ELSE)
  {
    Advance();

    if (m_Pos < m_Tokens.size() && CurrentToken().type == TOKEN_TYPE::IF)
    {
      auto false_scope = IfElseStatement();

      auto location = Range{
        .start = TokenStart(if_token),
        .end = false_scope->range.end
      };

      return std::make_unique<IfElseExpressionASTNode>(
        std::move(condition),
        std::move(true_scope),
        std::move(false_scope),
        m_NodeId++,
        location
      );
    }

    auto false_scope = Scope();

    auto location = Range{
      .start = TokenStart(if_token),
      .end = false_scope->range.end
    };

    return std::make_unique<IfElseExpressionASTNode>(
      std::move(condition),
      std::move(true_scope),
      std::move(false_scope),
      m_NodeId++,
      location
    );
  }

  auto location = Range{
    .start = TokenStart(if_token),
    .end = true_scope->range.end
  };

  return std::make_unique<IfElseExpressionASTNode>(
    std::move(condition),
    std::move(true_scope),
    nullptr,
    m_NodeId++,
    location
  );
}

auto Parser::ForLoop() -> ASTPtr
{
  const auto& for_token = CurrentToken();
  Advance();

  if (CurrentToken().type == TOKEN_TYPE::IDENTIFIER)
  {
    const auto& identifier = CurrentToken();
    Advance();

    Check(TOKEN_TYPE::COLON);
    Advance();

    auto over = Expression();
    auto scope = Scope();

    auto location = Range{
      .start = TokenStart(for_token),
      .end = scope->range.end
    };

    return std::make_unique<ForEachLoopASTNode>(
      identifier,
      std::move(over),
      std::move(scope),
      m_NodeId++,
      location
    );
  }

  auto declarations = VariableDeclaration();
  auto declaration = std::move(declarations[0]);
  auto condition = SemicolonExpression();
  auto increment = StatementOrIdentifier();
  auto scope = Scope();

  auto location = Range{
    .start = TokenStart(for_token),
    .end = scope->range.end
  };

  return std::make_unique<ForLoopASTNode>(
    std::move(declaration),
    std::move(condition),
    std::move(increment),
    std::move(scope),
    m_NodeId++,
    location
  );
}

auto Parser::WhileLoop() -> ASTPtr
{
  const auto& while_token = CurrentToken();
  Advance();

  auto condition = Expression();
  auto scope = Scope();

  auto location = Range{
    .start = TokenStart(while_token),
    .end = scope->range.end
  };

  return std::make_unique<WhileLoopASTNode>(
    std::move(condition),
    std::move(scope),
    m_NodeId++,
    location
  );
}

auto Parser::Class() -> ASTPtr
{
  const auto& class_token = CurrentToken();
  Advance();

  Check(TOKEN_TYPE::IDENTIFIER);
  const auto& id = CurrentToken();
  Advance();

  Check(TOKEN_TYPE::LBRACK);
  Advance();

  std::vector<std::unique_ptr<BaseASTNode>> fns;

  while (CurrentToken().type != TOKEN_TYPE::RBRACK)
  {
    fns.push_back(Function());
  }

  const auto& closing_brace = CurrentToken();
  Advance();

  auto location = LocationFromTokens(class_token, closing_brace);

  return std::make_unique<ClassASTNode>(
    id,
    std::move(fns),
    m_NodeId++,
    location
  );
}

auto Parser::Scope() -> ASTPtr
{
  Check(TOKEN_TYPE::LBRACK);
  const auto& open_brace = CurrentToken();
  Advance();

  auto scope = std::make_unique<ScopeASTNode>(
    m_NodeId++,
    LocationFromToken(open_brace)
  );

  while (m_Pos < m_Tokens.size())
  {
    switch (CurrentToken().type)
    {
      case TOKEN_TYPE::RBRACK:
      {
        const auto& close_brace = CurrentToken();
        Advance();
        scope->range = LocationFromTokens(open_brace, close_brace);
        return scope;
      }

      case TOKEN_TYPE::RETURN:
      {
        scope->nodes.push_back(Return());
        break;
      }

      case TOKEN_TYPE::CONST:
      case TOKEN_TYPE::LET:
      {
        auto vars = VariableDeclaration();
        for (auto &var : vars)
          scope->nodes.push_back(std::move(var));
        break;
      }

      case TOKEN_TYPE::IDENTIFIER:
      {
        scope->nodes.push_back(StatementOrIdentifier());
        break;
      }

      case TOKEN_TYPE::IF:
      {
        scope->nodes.push_back(IfElseStatement());
        break;
      }

      case TOKEN_TYPE::WHILE:
      {
        scope->nodes.push_back(WhileLoop());
        break;
      }

      case TOKEN_TYPE::FOR:
      {
        scope->nodes.push_back(ForLoop());
        break;
      }

      default:
      {
        scope->nodes.push_back(SemicolonExpression());
        break;
      }
    }
  }

  return scope;
}

auto Parser::Function() -> ASTPtr
{
  auto decl = FunctionDeclaration();
  auto body = Scope();

  auto location = Range{
    .start = decl->range.start,
    .end = body->range.end
  };

  return std::make_unique<FunctionASTNode>(
    std::move(decl),
    std::move(body),
    m_NodeId++,
    location
  );
}

auto Parser::Parse() -> ASTPtr
{
  try
  {
    const auto start_token = CurrentToken();
    auto root = std::make_unique<RootASTNode>(
      m_NodeId++,
      LocationFromToken(start_token)
    );

    while (m_Pos < m_Tokens.size())
    {
      switch (CurrentToken().type)
      {
        case TOKEN_TYPE::TT_EOF:
        {
          if (!root->nodes.empty())
          {
            root->range = Range{
              .start = TokenStart(start_token),
              .end = root->nodes.back()->range.end
            };
          }
          else
          {
            root->range = LocationFromToken(start_token);
          }

          return root;
        }

        case TOKEN_TYPE::CONST:
        case TOKEN_TYPE::LET:
        {
          auto vars = VariableDeclaration();
          for (auto &var : vars)
            root->nodes.push_back(std::move(var));
          break;
        }

        case TOKEN_TYPE::IDENTIFIER:
        {
          root->nodes.push_back(StatementOrIdentifier());
          break;
        }

        case TOKEN_TYPE::FN:
        {
          root->nodes.push_back(Function());
          break;
        }

        case TOKEN_TYPE::IF:
        {
          root->nodes.push_back(IfElseStatement());
          break;
        }

        case TOKEN_TYPE::WHILE:
        {
          root->nodes.push_back(WhileLoop());
          break;
        }

        case TOKEN_TYPE::FOR:
        {
          root->nodes.push_back(ForLoop());
          break;
        }

        case TOKEN_TYPE::CLASS:
        {
          root->nodes.push_back(Class());
          break;
        }

        case TOKEN_TYPE::IMPORT:
        {
          root->nodes.push_back(Import());
          break;
        }

        case TOKEN_TYPE::EXPORT:
        {
          auto vars = Export();
          for (auto &var : vars)
            root->nodes.push_back(std::move(var));
          break;
        }

        default:
        {
          root->nodes.push_back(SemicolonExpression());
          break;
        }
      }
    }

    if (!root->nodes.empty())
    {
      root->range = Range{
        .start = TokenStart(start_token),
        .end = root->nodes.back()->range.end
      };
    }
    else if (!m_Tokens.empty())
    {
      root->range = LocationFromToken(start_token);
    }

    return root;
  }
  catch (const SyntaxError &error)
  {
    std::cerr << error.what() << std::endl;
    return nullptr;
  }
}


auto Parser::MakePosition(const std::size_t line, const std::size_t character) -> Position
{
	return Position{
		.line = line,
		.character = character
	};
}

auto Parser::TokenStart(const Token &token) -> Position
{
	return token.range.start;
}

auto Parser::TokenEnd(const Token &token) -> Position
{
  return token.range.end;
}

auto Parser::LocationFromToken(const Token &token) -> Range
{
	return token.range;
}

auto Parser::LocationFromTokens(const Token &start_token, const Token &end_token) -> Range
{
	return Range{
		.start = TokenStart(start_token),
		.end = TokenEnd(end_token)
	};
}

auto Parser::LocationFromNodes(const BaseASTNode &start_node, const BaseASTNode &end_node) -> Range
{
	return Range{
		.start = start_node.range.start,
		.end = end_node.range.end
	};
}

auto Parser::LocationFromTokenToNode(const Token &start_token, const BaseASTNode &end_node) -> Range
{
	return Range{
		.start = TokenStart(start_token),
		.end = end_node.range.end
	};
}

auto Parser::LocationFromNodeToToken(const BaseASTNode &start_node, const Token &end_token) -> Range
{
	return Range{
		.start = start_node.range.start,
		.end = TokenEnd(end_token)
	};
}

}
