//
// Created by lenin on 14.11.2024.
//

#include "yapl/Parser.hpp"
#include "yapl/Token.hpp"
#include <memory>

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
	advance();

    switch (token.type) {
        case TOKEN_TYPE::INTEGER: return std::make_unique<IntegerASTNode>(token);
        case TOKEN_TYPE::STRING:
        case TOKEN_TYPE::FSTRING: return std::make_unique<StringASTNode>(token);
        case TOKEN_TYPE::FLOAT: return std::make_unique<FloatASTNode>(token);
        case TOKEN_TYPE::BOOL: return std::make_unique<BooleanASTNode>(token);
        default: throw SyntaxError(
                    m_filename,
                    token.line,
                    token.col_start,
                    token.col_end,
                    m_source_lines[token.line-1],
                    std::format("Unexpected token {} when parsing literal.", ttype_to_string(token.type)));
    }
}


std::unique_ptr<BaseASTNode> Parser::parse_starred_expr_or_expr()
{
    if (m_tokens[m_pos].type == TOKEN_TYPE::TIMES)
    {
        advance();
        return std::make_unique<StarredExpressionASTNode>(std::move(parse_expr()));
    }
    return parse_expr();
}

std::unique_ptr<BaseASTNode> Parser::parse_function_call(Token identifier)
{
    std::vector<std::unique_ptr<BaseASTNode>> args;
    // function call;
    advance(); // eat (
    while (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
    {
//        args.push_back(std::move(parse_expr()));
        args.push_back(std::move(parse_starred_expr_or_expr()));
        if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
        {
            advance();
        }
        if (m_tokens[m_pos].type == TOKEN_TYPE::TT_EOF)
        {
            throw SyntaxError(
                m_filename,
                identifier.line,
                identifier.col_start,
                identifier.col_end,
                m_source_lines[identifier.line-1],
                std::format("Expected {} after function call.", ttype_to_string(TOKEN_TYPE::RPAREN))
            );
        }
    }
    advance(); // eat )
    return std::make_unique<FunctionCallASTNode>(std::make_unique<IdentifierASTNode>(identifier), args);
}

std::unique_ptr<BaseASTNode> Parser::parse_method_or_property_call(Token identifier)
{
    advance(); // eat .
    auto name = m_tokens[m_pos];
    advance(); // eat name

		if (m_tokens[m_pos].type != TOKEN_TYPE::LPAREN)
			return std::make_unique<GetPropertyASTNode>(std::make_unique<IdentifierASTNode>(identifier), name);

    std::vector<std::unique_ptr<BaseASTNode>> args;
    // function call;
    advance(); // eat (
    while (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
    {
        args.push_back(std::move(parse_expr()));
        if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
        {
            advance();
        }
    }
    advance(); // eat )
    return std::make_unique<MethodCallASTNode>(std::make_unique<IdentifierASTNode>(identifier), name, args);
}

std::unique_ptr<BaseASTNode> Parser::parse_indexing(Token identifier)
{
    advance(); // eat [
    auto expr = parse_expr(); // TODO: maybe move this to paren expr as well?
    advance(); // eat ]
    return std::make_unique<IndexASTNode>(std::make_unique<IdentifierASTNode>(identifier), std::move(expr));
}


std::unique_ptr<BaseASTNode> Parser::parse_identifier() 
{
	auto identifier = m_tokens[m_pos];
  advance();
	if (m_tokens[m_pos].type == TOKEN_TYPE::LPAREN)
		return parse_function_call(identifier);
	if (m_tokens[m_pos].type == TOKEN_TYPE::LSQBRACK)
		return parse_indexing(identifier);
	if (m_tokens[m_pos].type == TOKEN_TYPE::PERIOD)
		return parse_property_or_method_chain(std::make_unique<IdentifierASTNode>(identifier));

	auto res = std::make_unique<IdentifierASTNode>(identifier);
	return res;
}

std::unique_ptr<BaseASTNode> Parser::parse_array()
{
	advance(); // eat [
	std::vector<std::unique_ptr<BaseASTNode>> values;
	while (m_tokens[m_pos].type != TOKEN_TYPE::RSQBRACK)
	{
		auto expr = parse_expr();
		values.push_back(std::move(expr));
		if (m_tokens[m_pos].type == TOKEN_TYPE::RSQBRACK)
			break;

        check(TOKEN_TYPE::COMMA);
		advance(); // eat ,
	}

	advance(); // eat ]

	return std::make_unique<ArrayASTNode>(values);
}

std::unique_ptr<BaseASTNode> Parser::parse_dict()
{
	advance(); // {
	std::vector<std::unique_ptr<BaseASTNode>> keys, values;

	while (m_tokens[m_pos].type != TOKEN_TYPE::RBRACK)
	{
		auto key = parse_expr();
		check(TOKEN_TYPE::COLON);
		advance();
		auto val = parse_expr();

		keys.push_back(std::move(key));
		values.push_back(std::move(val));

		if (m_tokens[m_pos].type == TOKEN_TYPE::RBRACK)
			break;

		check(TOKEN_TYPE::COMMA);
		advance();
	}
	advance(); // }

	return std::make_unique<DictASTNode>(std::move(keys), std::move(values));
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

	return std::make_unique<UnaryOpASTNode>(op, std::move(expr));
}

std::unique_ptr<BaseASTNode> Parser::parse_paren_expr()
{
	advance();
	auto expr = parse_expr();
	if (expr == nullptr)
		return nullptr;

    check(TOKEN_TYPE::RPAREN);
	advance();
    // if there is a [, then this is an indexing
    if (m_tokens[m_pos].type == TOKEN_TYPE::LSQBRACK)
    {
        advance(); // eat [
        auto index_expr = parse_expr();
        check(TOKEN_TYPE::RSQBRACK);
        advance(); // eat ]
        return std::make_unique<IndexASTNode>(std::move(expr), std::move(index_expr));
    }
    // if there is a ., then this is a method call
    if (m_tokens[m_pos].type == TOKEN_TYPE::PERIOD)
    {
        advance(); // eat .
        auto name = m_tokens[m_pos];
        check(TOKEN_TYPE::IDENTIFIER);
        advance(); // eat name

				if (m_tokens[m_pos].type != TOKEN_TYPE::LPAREN)
					return std::make_unique<GetPropertyASTNode>(std::move(expr), name);

        std::vector<std::unique_ptr<BaseASTNode>> args;
        // function call;
        check(TOKEN_TYPE::LPAREN);
        advance(); // eat (
        while (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
        {
            args.push_back(std::move(parse_expr()));
            if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
            {
                advance();
            }
        }
        advance(); // eat )
        return std::make_unique<MethodCallASTNode>(std::move(expr), name, args);
    }

	return expr;
}

std::unique_ptr<BaseASTNode> Parser::parse_expr()
{
	// TODO: change
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


std::unique_ptr<BaseASTNode> Parser::parse_binop_rhs(int expr_prec, std::unique_ptr<BaseASTNode> LHS)
{
	// tokens that arent operators have precedence of -1, so if expr doesnt have a RHS, we will just exit the loop
	while (true)
	{
		auto token_prec = get_token_precedence(m_tokens[m_pos]);
		if (token_prec < expr_prec)
			return LHS;

		auto op = m_tokens[m_pos];
		advance();

		auto RHS = parse_primary_expr();
		if (!RHS)
			return nullptr;

		// if next op has higher prec, we let it take the current rhs as its lhs
		// 1 + 2 * 3
		// LHS = 1
		// OP = + ; prec = 20
		// RHS = 2
		// NEXT_OP = * ; prec = 40
		// => we should pass 2 to * operator

		int next_prec = get_token_precedence(m_tokens[m_pos]);
		if (token_prec < next_prec)
		{
			RHS = parse_binop_rhs(token_prec + 1, std::move(RHS));
			if (RHS == nullptr)
				return nullptr;
		}

		LHS = std::make_unique<BinaryOpASTNode>(op, std::move(LHS), std::move(RHS));
	}
}

std::vector<std::unique_ptr<BaseASTNode>> Parser::parse_variable_declaration()
{
	std::vector<std::unique_ptr<BaseASTNode>> ret_val;
	auto decl_token = m_tokens[m_pos];
	advance(); // eat var/let/const
	while (true)
	{
        if (m_tokens[m_pos].type != TOKEN_TYPE::IDENTIFIER)
        {
            // TODO: this probably works wrong with multiline stuff
            throw SyntaxError(
                    m_filename,
                    decl_token.line,
                    decl_token.col_start,
                    m_tokens[m_pos].col_end,
                    m_source_lines[decl_token.line-1],
                    std::format("Expected identifier after variable declaration, but instead got {}", ttype_to_string(m_tokens[m_pos].type))
            );
        }
		auto name_identifier = m_tokens[m_pos];
        advance(); // eat identifier

		if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
		{
			advance(); // eat =
			auto expr = parse_expr();
			ret_val.push_back(std::move(std::make_unique<VariableASTNode>(decl_token, name_identifier, std::move(expr))));

			if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
			{
				advance(); // eat comma
				continue;
			}
			if (m_tokens[m_pos].type == TOKEN_TYPE::SEMICOLON)
			{
				advance(); // eat ;
				return ret_val;
			}
            throw SyntaxError(
                    m_filename,
                    decl_token.line,
                    decl_token.col_start,
                    decl_token.col_start, // TODO: add method to get span from all the nodes, something like expr.colspan();
                    m_source_lines[decl_token.line-1],
                    std::format("Expected semicolon after variable declaration")
            );
		}
		ret_val.push_back(std::move(std::make_unique<VariableASTNode>(decl_token, name_identifier)));
		if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
		{
			advance(); // eat comma
			continue;
		}
		if (m_tokens[m_pos].type == TOKEN_TYPE::SEMICOLON)
		{
			advance(); // eat ;
			return ret_val;
		}

        throw SyntaxError(
                m_filename,
                decl_token.line,
                decl_token.col_start,
                name_identifier.col_end,
                m_source_lines[decl_token.line-1],
                std::format("Expected semicolon after variable declaration")
        );
	}
}


std::unique_ptr<BaseASTNode> Parser::parse_method_call(std::unique_ptr<BaseASTNode> identifier)
{
	// Line.make().print()
	auto name = m_tokens[m_pos]; // make
	advance();
	check(TOKEN_TYPE::LPAREN);
	advance(); // eat (

	std::vector<std::unique_ptr<BaseASTNode>> args;
	while (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
	{
		args.push_back(std::move(parse_expr()));
		if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
		{
			advance();
		}
	}
	advance(); // eat )

	return std::make_unique<MethodCallASTNode>(std::move(identifier), name, args);
}

std::unique_ptr<BaseASTNode> Parser::parse_property_get(std::unique_ptr<BaseASTNode> identifier)
{
	auto name = m_tokens[m_pos];
	advance(); // name

	if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
	{
		advance(); // eat =
		auto expr = parse_expr();

		return std::make_unique<SetPropertyASTNode>(std::move(identifier), name, std::move(expr));
	}

	return std::make_unique<GetPropertyASTNode>(std::move(identifier), name);
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
	const auto& identifier = m_tokens[m_pos];
	advance(); // eat id
	if (m_tokens[m_pos].type == TOKEN_TYPE::LPAREN)
	{
        auto expr = parse_function_call(identifier);
        check(TOKEN_TYPE::SEMICOLON);
        advance();
        return expr;
	}
	if (m_tokens[m_pos].type == TOKEN_TYPE::PERIOD) // method call or property set
	{
		auto chain = parse_property_or_method_chain(std::make_unique<IdentifierASTNode>(identifier));
		check(TOKEN_TYPE::SEMICOLON);
		advance();
		return chain;
		advance(); // eat .
    auto name = m_tokens[m_pos];
    advance(); // eat name

		if (m_tokens[m_pos].type != TOKEN_TYPE::LPAREN)
		{
			if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
			{
				advance(); // eat eq
				auto expr = parse_semic_expr();
				return std::make_unique<SetPropertyASTNode>(std::make_unique<IdentifierASTNode>(identifier), name, std::move(expr));
			}
			check(TOKEN_TYPE::SEMICOLON);
			advance(); // eat ;
			return std::make_unique<GetPropertyASTNode>(std::make_unique<IdentifierASTNode>(identifier), name);
		}

    std::vector<std::unique_ptr<BaseASTNode>> args;
    // function call;
    advance(); // eat (
    while (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
    {
        args.push_back(std::move(parse_expr()));
        if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
        {
            advance();
        }
    }
    advance(); // eat )
    auto method = std::make_unique<MethodCallASTNode>(std::make_unique<IdentifierASTNode>(identifier), name, args);
		if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
		{
			advance(); // eat eq
			auto expr = parse_semic_expr();

			auto stmnt = std::make_unique<StatementASTNode>(std::move(method), std::move(expr));
			return std::move(stmnt);
		}
		check(TOKEN_TYPE::SEMICOLON);
		advance(); // eat ;
		return std::move(method);
	}
	if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
	{
		advance(); // eat eq
		auto expr = parse_semic_expr();

		auto stmnt = std::make_unique<StatementASTNode>(std::make_unique<IdentifierASTNode>(identifier), std::move(expr));
		return std::move(stmnt);
	}
	if (m_tokens[m_pos].type == TOKEN_TYPE::PLUSEQ
			|| m_tokens[m_pos].type == TOKEN_TYPE::MINUSEQ
			|| m_tokens[m_pos].type == TOKEN_TYPE::TIMESEQ
			|| m_tokens[m_pos].type == TOKEN_TYPE::MODEQ
			|| m_tokens[m_pos].type == TOKEN_TYPE::SLASHEQ)
	{
			static std::unordered_map<TOKEN_TYPE, TOKEN_TYPE> tt_to_tt = {
							{TOKEN_TYPE::PLUSEQ, TOKEN_TYPE::PLUS},
							{TOKEN_TYPE::MINUSEQ, TOKEN_TYPE::MINUS},
							{TOKEN_TYPE::TIMESEQ, TOKEN_TYPE::TIMES},
							{TOKEN_TYPE::MODEQ, TOKEN_TYPE::MOD},
							{TOKEN_TYPE::SLASHEQ, TOKEN_TYPE::SLASH},
			};
			auto new_token = tt_to_tt[m_tokens[m_pos].type];
			advance(); // eat pluseq
			auto expr = parse_semic_expr();
			auto expanded_expr = std::make_unique<BinaryOpASTNode>(Token{new_token}, std::make_unique<IdentifierASTNode>(identifier), std::move(expr));

			auto stmnt = std::make_unique<StatementASTNode>(std::make_unique<IdentifierASTNode>(identifier), std::move(expanded_expr));
			return std::move(stmnt);
	}
	if (m_tokens[m_pos].type == TOKEN_TYPE::LSQBRACK) // then this is a[1] = 5; or just a[1];
	{
		m_pos--;
		auto idx = parse_identifier();
        if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
        {
            advance(); // eat =
            auto expr = parse_semic_expr();
            return std::make_unique<StatementIndexASTNode>(std::move(idx), std::move(expr));
        }
        check(TOKEN_TYPE::SEMICOLON);
        advance();
        return idx;
	}
	if (m_tokens[m_pos].type == TOKEN_TYPE::SEMICOLON)
	{
		advance(); // eat ;
		return std::make_unique<IdentifierASTNode>(identifier);
	}

	m_pos--;
	return parse_semic_expr();

}

std::unique_ptr<BaseASTNode> Parser::parse_import()
{
	check(TOKEN_TYPE::IMPORT);
	advance(); // eat import

	check(TOKEN_TYPE::LBRACK);
	advance(); // eat {

	std::vector<Token> identifiers;

	while (m_tokens[m_pos].type != TOKEN_TYPE::RBRACK)
	{
		check(TOKEN_TYPE::IDENTIFIER);
		auto id = m_tokens[m_pos];
		identifiers.push_back(id);
		advance();

		if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
		{
			advance(); // eat ,
		}
	}

	advance(); // eat }

	check(TOKEN_TYPE::FROM);
	advance(); // eat from

	check(TOKEN_TYPE::STRING);
	auto module = parse_literal();

	check(TOKEN_TYPE::SEMICOLON);
	advance();

	return std::make_unique<ImportASTNode>(std::move(identifiers), std::move(module));
}

std::unique_ptr<BaseASTNode> Parser::parse_export()
{
	// TODO: allow more variants for export (e.g. inline export fn ...)
	check(TOKEN_TYPE::EXPORT);
	advance(); // eat export

	check(TOKEN_TYPE::LBRACK);
	advance(); // eat {

	std::vector<std::unique_ptr<BaseASTNode>> vars;
	while (m_tokens[m_pos].type != TOKEN_TYPE::RBRACK)
	{
		check(TOKEN_TYPE::IDENTIFIER);
		auto id = m_tokens[m_pos];
		advance(); // eat id

		vars.push_back(std::make_unique<IdentifierASTNode>(id));

		if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
		{
			advance(); // eat ,
		}

	}

	check(TOKEN_TYPE::RBRACK);
	advance(); // eat }

	return std::make_unique<ExportASTNode>(std::move(vars));
}

std::unique_ptr<BaseASTNode> Parser::parse_function_arguments()
{
    check(TOKEN_TYPE::LPAREN);
	advance(); // eat (
	std::vector<std::unique_ptr<FunctionArgumentASTNode>> args;
    std::unique_ptr<FunctionArgumentASTNode> args_arg, kwargs_arg;
    bool has_args = false;
    bool has_kwargs = false;
	while (m_pos < m_tokens.size())
	{
		if (m_tokens[m_pos].type == TOKEN_TYPE::RPAREN)
		{
			advance(); // eat )
			break;
		}
        // check if args
        if (m_tokens[m_pos].type == TOKEN_TYPE::TIMES)
        {
            // check if kwargs
            if (m_tokens[m_pos+1].type == TOKEN_TYPE::TIMES)
            {
                // cannot have multiple kwargs
                if (has_kwargs)
                    throw SyntaxError(
                            m_filename,
                            m_tokens[m_pos+1].line,
                            m_tokens[m_pos].col_start,
                            m_tokens[m_pos+1].col_start,
                            m_source_lines[m_tokens[m_pos+1].line-1],
                            "Cannot have multiple **kwargs"
                    );
                advance(); // eat *
                advance(); // eat *

                check(TOKEN_TYPE::IDENTIFIER);
                Token identifier = m_tokens[m_pos];
                advance(); // eat identifier
                has_kwargs = true;
                kwargs_arg = std::make_unique<FunctionArgumentASTNode>(identifier,
                                                                                 Token{ TOKEN_TYPE::IDENTIFIER,
                                                                                        new char[]{ "any" } },
                                                                                 false, true);
                if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
                {
                    advance(); // eat comma
                }
                continue;
            }

            // cannot have multiple args
            if (has_args)
                throw SyntaxError(
                        m_filename,
                        m_tokens[m_pos].line,
                        m_tokens[m_pos].col_start,
                        m_tokens[m_pos].col_start,
                        m_source_lines[m_tokens[m_pos].line-1],
                        "Cannot have multiple *args"
                );
            advance(); // eat *

            check(TOKEN_TYPE::IDENTIFIER);
            Token identifier = m_tokens[m_pos];
            advance(); // eat identifier
            has_args = true;
            args_arg = std::make_unique<FunctionArgumentASTNode>(identifier,
                                                                           Token{ TOKEN_TYPE::IDENTIFIER,
                                                                                  new char[]{ "any" } },
                                                                           true, false);
            if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA)
            {
                advance(); // eat comma
            }
            continue;
        }
		// parse an argument
		Token identifier = m_tokens[m_pos];
        check(TOKEN_TYPE::IDENTIFIER);
		advance(); // eat ident
        /*
        check(TOKEN_TYPE::COLON);
        advance(); // eat :
        Token type = m_tokens[m_pos];
        check(TOKEN_TYPE::IDENTIFIER);
        advance(); // eat type
         */
		if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA) 
		{
			advance(); // eat comma
		}

		args.push_back(std::move(std::make_unique<FunctionArgumentASTNode>(identifier, Token{ TOKEN_TYPE::IDENTIFIER, new char[]{ "any" } })));
	}
	return std::move(std::make_unique<FunctionArgumentListASTNode>(args, std::move(args_arg), std::move(kwargs_arg)));
}

std::unique_ptr<BaseASTNode> Parser::parse_function_declaration()
{
	advance(); // eat fn
	auto fname = m_tokens[m_pos];
    check(TOKEN_TYPE::IDENTIFIER);
	advance(); // eat identifier
	auto fargs = parse_function_arguments();
    /*
    check(TOKEN_TYPE::COLON);
	advance(); // eat :
	auto ftype = m_tokens[m_pos];
    check(TOKEN_TYPE::IDENTIFIER);
	advance(); // eat type
    */

	return std::move(std::make_unique<FunctionDeclASTNode>(fname, std::move(fargs), Token{ TOKEN_TYPE::IDENTIFIER, new char[]{ "any" } }));
}

std::unique_ptr<BaseASTNode> Parser::parse_return() 
{
	advance(); // eat return;
	auto expr = parse_semic_expr();
	auto ret = std::make_unique<ReturnStatementASTNode>(std::move(expr));
	return std::move(ret);
}

std::unique_ptr<BaseASTNode> Parser::parse_ifelse_statement()
{
    const auto& if_token = m_tokens[m_pos];
	advance(); // eat if
	auto condition = parse_expr();
    // TODO: tidy up
    if (m_tokens[m_pos].type != TOKEN_TYPE::LBRACK)
    {
        const auto& current_token = m_tokens[m_pos];
        throw SyntaxError(
                m_filename,
                if_token.line,
                if_token.col_start,
                if_token.col_end, // FIXME: this really should be the end of the condition, but lets leave it like that for now
                m_source_lines[if_token.line-1],
                std::format("Expected token {} after if-statement condition", ttype_to_string(TOKEN_TYPE::LBRACK))
        );
    }
	auto true_scope = parse_scope();
	if (m_pos < m_tokens.size() && m_tokens[m_pos].type == TOKEN_TYPE::ELSE)
	{
		advance(); // eat else
		if (m_pos < m_tokens.size() && m_tokens[m_pos].type == TOKEN_TYPE::IF)
		{
			// this is another if statement then
			auto false_scope = parse_ifelse_statement();
			return std::make_unique<IfElseExpressionASTNode>(std::move(condition), std::move(true_scope), std::move(false_scope));
		}
		auto false_scope = parse_scope();
		return std::make_unique<IfElseExpressionASTNode>(std::move(condition), std::move(true_scope), std::move(false_scope));
	}
	// if no else block
	return std::make_unique<IfElseExpressionASTNode>(std::move(condition), std::move(true_scope));
}

std::unique_ptr<BaseASTNode> Parser::parse_for_loop()
{
	advance(); // eat for token

	// for-each
	if (m_tokens[m_pos].type == TOKEN_TYPE::IDENTIFIER)
	{
		auto identifier = m_tokens[m_pos];
		advance();
		check(TOKEN_TYPE::COLON);
		advance(); // eat :
		auto over = parse_expr();
		auto scope = parse_scope();

		return std::make_unique<ForEachLoopASTNode>(identifier, std::move(over), std::move(scope));
	}
	auto declaration = std::move(parse_variable_declaration()[0]);
	auto condition = parse_semic_expr();
	auto increment = parse_statement_or_ident();
	auto scope = parse_scope();
	return std::make_unique<ForLoopASTNode>(std::move(declaration), std::move(condition), std::move(increment), std::move(scope));
}

std::unique_ptr<BaseASTNode> Parser::parse_while_loop()
{
	advance(); // eat while token
	auto condition = parse_expr();
	auto scope = parse_scope();
	return std::make_unique<WhileLoopASTNode>(std::move(condition), std::move(scope));
}

std::unique_ptr<BaseASTNode> Parser::parse_class()
{
	advance(); // eat class

	check(TOKEN_TYPE::IDENTIFIER);
	auto id = m_tokens[m_pos];
	advance();

	check(TOKEN_TYPE::LBRACK);
	advance();

	std::vector<std::unique_ptr<BaseASTNode>> fns;
	// inside the class atm there can be only functions
	// TODO: add getters, setters later
	while (m_tokens[m_pos].type != TOKEN_TYPE::RBRACK)
	{
		fns.push_back(parse_function());
	}
	advance(); // eat }
	return std::make_unique<ClassASTNode>(id, std::move(fns));
}


std::unique_ptr<BaseASTNode> Parser::parse_scope()
{
	// todo: add checks
	// parsing a scope
    check(TOKEN_TYPE::LBRACK);
	advance(); // eat {
	auto scope = std::make_unique<ScopeASTNode>();
	while (m_pos < m_tokens.size())
	{
		switch (m_tokens[m_pos].type)
		{
			case TOKEN_TYPE::RBRACK: { advance(); return scope; }
			case TOKEN_TYPE::RETURN: { scope->nodes.push_back(std::move(parse_return())); break; }
			case TOKEN_TYPE::VAR:
			case TOKEN_TYPE::CONST:
			case TOKEN_TYPE::LET:
			{
				auto vars = parse_variable_declaration();
				for (auto& var : vars)
					scope->nodes.push_back(std::move(var));
				break;
			}
			case TOKEN_TYPE::IDENTIFIER: { scope->nodes.push_back(std::move(parse_statement_or_ident())); break; } // TODO: handle func calls
			// case TOKEN_TYPE::FN: { scope->nodes.push_back(std::move(parse_function())); break; }
			case TOKEN_TYPE::IF: { scope->nodes.push_back(std::move(parse_ifelse_statement())); break; }
			case TOKEN_TYPE::WHILE: { scope->nodes.push_back(std::move(parse_while_loop())); break; }
			case TOKEN_TYPE::FOR: { scope->nodes.push_back(std::move(parse_for_loop())); break; }
			default: { scope->nodes.push_back(std::move(parse_semic_expr())); break; }
		}
		// if (m_pos != m_tokens.size() && m_tokens[m_pos].type != TOKEN_TYPE::SEMICOLON)
		// {
		// 	std::cout << "Excpected a semicolon;\n";
		// 	return nullptr;
		// }
		// advance(); // eat ;
	}
	return scope;
}

std::unique_ptr<BaseASTNode> Parser::parse_function()
{
	auto decl = parse_function_declaration();
	auto body = parse_scope();

	return std::move(std::make_unique<FunctionASTNode>(std::move(decl), std::move(body)));
}

// this is sorta wrong, change later
std::unique_ptr<BaseASTNode> Parser::parse_root()
{
    try
    {

        auto root = std::make_unique<RootASTNode>();
        while (m_pos < m_tokens.size())
        {
            switch (m_tokens[m_pos].type)
            {
                case TOKEN_TYPE::TT_EOF: { return root; }
                case TOKEN_TYPE::VAR:
                case TOKEN_TYPE::CONST:
                case TOKEN_TYPE::LET:
                {
                    auto vars = parse_variable_declaration();
                    for (auto& var : vars)
                        root->nodes.push_back(std::move(var));
                    break;
                }
                case TOKEN_TYPE::IDENTIFIER: { root->nodes.push_back(std::move(parse_statement_or_ident())); break; } // TODO: handle func calls
                case TOKEN_TYPE::FN: { root->nodes.push_back(std::move(parse_function())); break; }
                case TOKEN_TYPE::IF: { root->nodes.push_back(std::move(parse_ifelse_statement())); break; }
                case TOKEN_TYPE::WHILE: { root->nodes.push_back(std::move(parse_while_loop())); break; }
                case TOKEN_TYPE::FOR: { root->nodes.push_back(std::move(parse_for_loop())); break; }
								case TOKEN_TYPE::CLASS: { root->nodes.push_back(std::move(parse_class())); break; }
								case TOKEN_TYPE::IMPORT: { root->nodes.push_back(std::move(parse_import())); break; }
								case TOKEN_TYPE::EXPORT: { root->nodes.push_back(std::move(parse_export())); break; }
                default: { root->nodes.push_back(std::move(parse_semic_expr())); break; }
            }
        }
        return root;
    }
    catch (const SyntaxError& e)
    {
        std::cerr << e.what() << std::endl;
        return nullptr;
    }
}

}
