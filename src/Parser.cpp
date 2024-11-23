//
// Created by lenin on 14.11.2024.
//

#include "Parser.hpp"
#include "Token.hpp"
#include <memory>

namespace yapl {

void Parser::advance()
{
  m_pos++;
}


std::unique_ptr<BaseASTNode> Parser::parse_literal()
{
	auto res = std::make_unique<LiteralASTNode>(m_tokens[m_pos]);
	advance();
	return res;
}

std::unique_ptr<BaseASTNode> Parser::parse_identifier() 
{
	// auto res = std::make_unique<IdentifierASTNode>(m_tokens[m_pos]);
	auto identifier = m_tokens[m_pos];
	advance(); // eat identifier
	// if there is a (, then this is a function call
	if (m_tokens[m_pos].type == TOKEN_TYPE::LPAREN)
	{
		// std::cout << "is function call";
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
		return std::make_unique<FunctionCallASTNode>(identifier, args);
	}
	// if there is a [, then this is an indexing
	if (m_tokens[m_pos].type == TOKEN_TYPE::LSQBRACK)
	{
		advance(); // eat [
		auto expr = parse_expr(); // TODO: maybe move this to paren expr as well?
		advance(); // eat ]
		return std::make_unique<IndexASTNode>(std::make_unique<IdentifierASTNode>(identifier), std::move(expr));
	}
	// if there is a ., then this is a method call
	if (m_tokens[m_pos].type == TOKEN_TYPE::PERIOD)
	{
		advance(); // eat .
		auto name = m_tokens[m_pos];
		advance(); // eat name

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
		return std::make_unique<MethodCallASTNode>(identifier, name, args);
	}
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

		advance(); // eat ,
	}

	advance(); // eat ]

	return std::make_unique<ArrayASTNode>(values);
}


std::unique_ptr<BaseASTNode> Parser::parse_primary_expr()
{
	switch (m_tokens[m_pos].type)
	{
		default:
		{
			std::cout << "Unknown token when expecting an expression!\n";
			advance(); // eat unknown token
			return nullptr;
		}
		case TOKEN_TYPE::INTEGER:
		case TOKEN_TYPE::FLOAT:
		case TOKEN_TYPE::STRING:
		case TOKEN_TYPE::FSTRING:
		case TOKEN_TYPE::BOOL: { return parse_literal(); }
		case TOKEN_TYPE::IDENTIFIER: { return parse_identifier(); }
		case TOKEN_TYPE::LPAREN: { return parse_paren_expr(); }
		case TOKEN_TYPE::LSQBRACK: { return parse_array(); }
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

	if (m_tokens[m_pos].type != TOKEN_TYPE::RPAREN)
	{
		// TODO: add better logger
		std::cerr << "Expected a ) token at " << __FILE__ << " " <<  __LINE__ << " " << __func__ << "\n";
		return nullptr;
	}
	advance();
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
	if (m_tokens[m_pos].type != TOKEN_TYPE::SEMICOLON)
	{
		std::cerr << "Expected a semicolon after an expression!\n";
		return nullptr;
	}
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

std::vector<std::unique_ptr<BaseASTNode>> Parser::parse_var_decl()
{
	std::vector<std::unique_ptr<BaseASTNode>> ret_val;
	auto decl_token = m_tokens[m_pos];
	advance(); // eat var/let/const
	while (true)
	{
		auto name_identifier = m_tokens[m_pos];
		advance(); // eat name
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
			std::cerr << "expected a semicolon in var decl\n";
			return ret_val;
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
	}
	return ret_val;
}



std::unique_ptr<BaseASTNode> Parser::parse_statement_or_ident()
{
	const auto& identifier = m_tokens[m_pos];
	advance(); // eat id
	if (m_tokens[m_pos].type == TOKEN_TYPE::LPAREN)
	{
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
		if (m_tokens[m_pos].type != TOKEN_TYPE::SEMICOLON)
		{
			std::cerr << "Expected a semicolon after statement\n";
			return nullptr;
		}
		advance(); // eat ;
		// for (const auto& a : args) std::cout << a->print() <<" ";
		return std::make_unique<FunctionCallASTNode>(identifier, args);
	}
	if (m_tokens[m_pos].type == TOKEN_TYPE::ASSIGN)
	{
		advance(); // eat eq
		auto expr = parse_semic_expr();

		std::unique_ptr<StatementASTNode> stmnt = std::make_unique<StatementASTNode>(identifier, std::move(expr));
		return std::move(stmnt);
	}
	if (m_tokens[m_pos].type == TOKEN_TYPE::LSQBRACK) // then this is a[1] = 5;
	{
		m_pos--;
		auto idx = parse_identifier();
		advance(); // eat =
		auto expr = parse_semic_expr();
		return std::make_unique<StatementIndexASTNode>(std::move(idx), std::move(expr));
	}
	if (m_tokens[m_pos].type == TOKEN_TYPE::SEMICOLON)
	{
		advance(); // eat ;
		return std::make_unique<IdentifierASTNode>(identifier);
	}

	m_pos--;
	return parse_semic_expr();

}

std::unique_ptr<BaseASTNode> Parser::parse_function_arguments()
{
	if (m_tokens[m_pos].type != TOKEN_TYPE::LPAREN)
	{
		std::cout << "Expected ( after function identifier\n";
		return nullptr;
	}
	advance(); // eat (
	std::vector<std::unique_ptr<FunctionArgumentASTNode>> args;
	while (m_pos < m_tokens.size())
	{
		if (m_tokens[m_pos].type == TOKEN_TYPE::RPAREN)
		{
			advance(); // eat )
			break;
		}
		// parse an argument
		Token identifier = m_tokens[m_pos];
		if (identifier.type != TOKEN_TYPE::IDENTIFIER)
		{
			std::cout << "Expected argument name\n";
			return nullptr;
		}
		advance(); // eat ident
		if (m_tokens[m_pos].type != TOKEN_TYPE::COLON)
		{
			std::cout << "Expected a colon\n";
			return nullptr;
		}
		advance(); // eat :
		Token type = m_tokens[m_pos];
		if (type.type != TOKEN_TYPE::IDENTIFIER)
		{
			std::cout << "Expected a type identifier\n";
			return nullptr;
		}
		advance(); // eat type
		if (m_tokens[m_pos].type == TOKEN_TYPE::COMMA) 
		{
			advance(); // eat comma
		}

		args.push_back(std::move(std::make_unique<FunctionArgumentASTNode>(identifier, type)));
	}
	return std::move(std::make_unique<FunctionArgumentListASTNode>(args));
}

std::unique_ptr<BaseASTNode> Parser::parse_function_declaration()
{
	advance(); // eat fn
	auto fname = m_tokens[m_pos];
	if (fname.type != TOKEN_TYPE::IDENTIFIER)
	{
		std::cout << "Expected an identifier after fn keyword\n";
		return nullptr;
	}
	advance(); // eat identifier
	// std::cout << fname.value << " ";
	auto fargs = parse_function_arguments();
	if (fargs == nullptr)
	{
		std::cout << "Error parsing arguments";
		return nullptr;
	}
	advance(); // eat :
	// TODO: add checks later
	auto ftype = m_tokens[m_pos];
	advance(); // eat type

	return std::move(std::make_unique<FunctionDeclASTNode>(fname, std::move(fargs), ftype));
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
	advance(); // eat if
	auto condition = parse_expr();
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
	auto declaration = std::move(parse_var_decl()[0]);
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

std::unique_ptr<BaseASTNode> Parser::parse_scope()
{
	// todo: add checks
	// parsing a scope
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
				auto vars = parse_var_decl();
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
	std::cout << "parsing func\n";
	auto decl = parse_function_declaration();
	auto body = parse_scope();

	return std::move(std::make_unique<FunctionASTNode>(std::move(decl), std::move(body)));
}

// this is sorta wrong, change later
std::unique_ptr<BaseASTNode> Parser::parse_root()
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
				auto vars = parse_var_decl();
				for (auto& var : vars)
					root->nodes.push_back(std::move(var));
				break;
			}
			case TOKEN_TYPE::IDENTIFIER: { root->nodes.push_back(std::move(parse_statement_or_ident())); break; } // TODO: handle func calls
			case TOKEN_TYPE::FN: { root->nodes.push_back(std::move(parse_function())); break; }
			case TOKEN_TYPE::IF: { root->nodes.push_back(std::move(parse_ifelse_statement())); break; }
			case TOKEN_TYPE::WHILE: { root->nodes.push_back(std::move(parse_while_loop())); break; }
			case TOKEN_TYPE::FOR: { root->nodes.push_back(std::move(parse_for_loop())); break; }
			default: { root->nodes.push_back(std::move(parse_semic_expr())); break; }
		}
		// if (m_pos != m_tokens.size() && m_tokens[m_pos].type != TOKEN_TYPE::SEMICOLON)
		// {
		// 	std::cout << "Excpected a semicolon;\n";
		// 	return nullptr;
		// }
		// advance(); // eat ;
	}
	return root;
}

}
