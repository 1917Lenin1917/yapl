#include <string>
#include <sstream>
#include <memory>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "yapl/Lexer.hpp"
#include "yapl/Parser.hpp"
#include "yapl/ASTNode.hpp"
#include "yapl/Token.hpp"

using namespace yapl;

static auto get_lines_from_text(const std::string& text) -> std::vector<std::string>
{
    std::vector<std::string> lines;
    std::stringstream stream{text};

    while (!stream.eof())
    {
        std::string line;
        std::getline(stream, line);
        lines.push_back(line);
    }

    return lines;
}

static auto parse_text(const std::string& text) -> ASTPtr
{
    Lexer lexer{text};
    Parser parser{lexer.Tokenize(), "<test_case>", get_lines_from_text(text)};
    return parser.Parse();
}

static auto require_root(ASTPtr& ast) -> RootASTNode*
{
    REQUIRE(ast != nullptr);
    auto* root = dynamic_cast<RootASTNode*>(ast.get());
    REQUIRE(root != nullptr);
    return root;
}

TEST_CASE("parser builds correct precedence tree for arithmetic expression", "[parser]")
{
    auto ast = parse_text("4 + 2 * 6 - (1 + 2) / 8;");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* minus = dynamic_cast<BinaryOpASTNode*>(root->nodes[0].get());
    REQUIRE(minus != nullptr);
    REQUIRE(minus->op.type == TOKEN_TYPE::MINUS);

    auto* minus_lhs = dynamic_cast<BinaryOpASTNode*>(minus->LHS.get());
    auto* minus_rhs = dynamic_cast<BinaryOpASTNode*>(minus->RHS.get());

    REQUIRE(minus_lhs != nullptr);
    REQUIRE(minus_rhs != nullptr);

    REQUIRE(minus_lhs->op.type == TOKEN_TYPE::PLUS);
    REQUIRE(minus_rhs->op.type == TOKEN_TYPE::SLASH);

    auto* plus_lhs = dynamic_cast<IntegerASTNode*>(minus_lhs->LHS.get());
    auto* plus_rhs = dynamic_cast<BinaryOpASTNode*>(minus_lhs->RHS.get());

    REQUIRE(plus_lhs != nullptr);
    REQUIRE(plus_rhs != nullptr);
    REQUIRE(plus_lhs->value == 4);
    REQUIRE(plus_rhs->op.type == TOKEN_TYPE::TIMES);

    auto* times_lhs = dynamic_cast<IntegerASTNode*>(plus_rhs->LHS.get());
    auto* times_rhs = dynamic_cast<IntegerASTNode*>(plus_rhs->RHS.get());

    REQUIRE(times_lhs != nullptr);
    REQUIRE(times_rhs != nullptr);
    REQUIRE(times_lhs->value == 2);
    REQUIRE(times_rhs->value == 6);

    auto* slash_lhs = dynamic_cast<BinaryOpASTNode*>(minus_rhs->LHS.get());
    auto* slash_rhs = dynamic_cast<IntegerASTNode*>(minus_rhs->RHS.get());

    REQUIRE(slash_lhs != nullptr);
    REQUIRE(slash_rhs != nullptr);
    REQUIRE(slash_lhs->op.type == TOKEN_TYPE::PLUS);
    REQUIRE(slash_rhs->value == 8);

    auto* nested_plus_lhs = dynamic_cast<IntegerASTNode*>(slash_lhs->LHS.get());
    auto* nested_plus_rhs = dynamic_cast<IntegerASTNode*>(slash_lhs->RHS.get());

    REQUIRE(nested_plus_lhs != nullptr);
    REQUIRE(nested_plus_rhs != nullptr);
    REQUIRE(nested_plus_lhs->value == 1);
    REQUIRE(nested_plus_rhs->value == 2);
}

TEST_CASE("parser builds unary expression correctly", "[parser]")
{
    auto ast = parse_text("-42;");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* unary = dynamic_cast<UnaryOpASTNode*>(root->nodes[0].get());
    REQUIRE(unary != nullptr);
    REQUIRE(unary->op.type == TOKEN_TYPE::MINUS);

    auto* value = dynamic_cast<IntegerASTNode*>(unary->RHS.get());
    REQUIRE(value != nullptr);
    REQUIRE(value->value == 42);
}

TEST_CASE("parser parses let declarations with and without initializer", "[parser]")
{
    auto ast = parse_text("let first = 1, second;");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 2);

    auto* first = dynamic_cast<VariableASTNode*>(root->nodes[0].get());
    auto* second = dynamic_cast<VariableASTNode*>(root->nodes[1].get());

    REQUIRE(first != nullptr);
    REQUIRE(second != nullptr);

    REQUIRE(first->type.type == TOKEN_TYPE::LET);
    REQUIRE(first->name.value == "first");
    REQUIRE(first->value != nullptr);

    auto* first_value = dynamic_cast<IntegerASTNode*>(first->value.get());
    REQUIRE(first_value != nullptr);
    REQUIRE(first_value->value == 1);

    REQUIRE(second->type.type == TOKEN_TYPE::LET);
    REQUIRE(second->name.value == "second");
    REQUIRE(second->value == nullptr);
}

TEST_CASE("parser parses plain function call with positional arguments", "[parser]")
{
    auto ast = parse_text("sum(1, 2, 3);");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* call = dynamic_cast<FunctionCallASTNode*>(root->nodes[0].get());
    REQUIRE(call != nullptr);

    auto* callee = dynamic_cast<IdentifierASTNode*>(call->base.get());
    REQUIRE(callee != nullptr);
    REQUIRE(callee->token.value == "sum");

    REQUIRE(call->args.size() == 3);

    auto* arg0 = dynamic_cast<IntegerASTNode*>(call->args[0].get());
    auto* arg1 = dynamic_cast<IntegerASTNode*>(call->args[1].get());
    auto* arg2 = dynamic_cast<IntegerASTNode*>(call->args[2].get());

    REQUIRE(arg0 != nullptr);
    REQUIRE(arg1 != nullptr);
    REQUIRE(arg2 != nullptr);

    REQUIRE(arg0->value == 1);
    REQUIRE(arg1->value == 2);
    REQUIRE(arg2->value == 3);
}

TEST_CASE("parser parses function call with keyword and starred arguments", "[parser]")
{
    auto ast = parse_text("fncall(1, named = 2, *rest);");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* call = dynamic_cast<FunctionCallASTNode*>(root->nodes[0].get());
    REQUIRE(call != nullptr);
    REQUIRE(call->args.size() == 3);

    auto* arg0 = dynamic_cast<IntegerASTNode*>(call->args[0].get());
    REQUIRE(arg0 != nullptr);
    REQUIRE(arg0->value == 1);

    auto* arg1 = dynamic_cast<KeyParamExpressionASTNode*>(call->args[1].get());
    REQUIRE(arg1 != nullptr);
    REQUIRE(arg1->identifier.value == "named");

    auto* arg1_expr = dynamic_cast<IntegerASTNode*>(arg1->expression.get());
    REQUIRE(arg1_expr != nullptr);
    REQUIRE(arg1_expr->value == 2);

    auto* arg2 = dynamic_cast<StarredExpressionASTNode*>(call->args[2].get());
    REQUIRE(arg2 != nullptr);

    auto* starred_expr = dynamic_cast<IdentifierASTNode*>(arg2->expression.get());
    REQUIRE(starred_expr != nullptr);
    REQUIRE(starred_expr->token.value == "rest");
}

TEST_CASE("parser parses property assignment statement", "[parser]")
{
    auto ast = parse_text("object.field = 10;");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* set_property = dynamic_cast<SetPropertyASTNode*>(root->nodes[0].get());
    REQUIRE(set_property != nullptr);
    REQUIRE(set_property->name.value == "field");

    auto* base = dynamic_cast<IdentifierASTNode*>(set_property->base_expr.get());
    REQUIRE(base != nullptr);
    REQUIRE(base->token.value == "object");

    auto* rhs = dynamic_cast<IntegerASTNode*>(set_property->RHS.get());
    REQUIRE(rhs != nullptr);
    REQUIRE(rhs->value == 10);
}

TEST_CASE("parser parses method/property chain statement", "[parser]")
{
    auto ast = parse_text("object.make().value;");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* property = dynamic_cast<GetPropertyASTNode*>(root->nodes[0].get());
    REQUIRE(property != nullptr);
    REQUIRE(property->name.value == "value");

    auto* method = dynamic_cast<MethodCallASTNode*>(property->base_expr.get());
    REQUIRE(method != nullptr);
    REQUIRE(method->name.value == "make");
    REQUIRE(method->args.empty());

    auto* method_base = dynamic_cast<IdentifierASTNode*>(method->base_expr.get());
    REQUIRE(method_base != nullptr);
    REQUIRE(method_base->token.value == "object");
}

TEST_CASE("parser parses indexing expression statement", "[parser]")
{
    auto ast = parse_text("items[3];");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* index = dynamic_cast<IndexASTNode*>(root->nodes[0].get());
    REQUIRE(index != nullptr);

    auto* base = dynamic_cast<IdentifierASTNode*>(index->base_expr.get());
    auto* idx = dynamic_cast<IntegerASTNode*>(index->index_expr.get());

    REQUIRE(base != nullptr);
    REQUIRE(idx != nullptr);

    REQUIRE(base->token.value == "items");
    REQUIRE(idx->value == 3);
}

TEST_CASE("parser parses indexing assignment statement", "[parser]")
{
    auto ast = parse_text("items[1] = 99;");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* statement = dynamic_cast<StatementIndexASTNode*>(root->nodes[0].get());
    REQUIRE(statement != nullptr);

    auto* index = dynamic_cast<IndexASTNode*>(statement->identifier.get());
    REQUIRE(index != nullptr);

    auto* base = dynamic_cast<IdentifierASTNode*>(index->base_expr.get());
    auto* idx = dynamic_cast<IntegerASTNode*>(index->index_expr.get());
    auto* rhs = dynamic_cast<IntegerASTNode*>(statement->RHS.get());

    REQUIRE(base != nullptr);
    REQUIRE(idx != nullptr);
    REQUIRE(rhs != nullptr);

    REQUIRE(base->token.value == "items");
    REQUIRE(idx->value == 1);
    REQUIRE(rhs->value == 99);
}

TEST_CASE("parser parses simple function declaration and return", "[parser]")
{
    auto ast = parse_text("fn add(a, b) { return a + b; }");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* function = dynamic_cast<FunctionASTNode*>(root->nodes[0].get());
    REQUIRE(function != nullptr);

    auto* decl = dynamic_cast<FunctionDeclASTNode*>(function->decl.get());
    REQUIRE(decl != nullptr);
    REQUIRE(decl->name.value == "add");

    auto* args = dynamic_cast<FunctionArgumentListASTNode*>(decl->args.get());
    REQUIRE(args != nullptr);
    REQUIRE(args->args.size() == 2);
    REQUIRE(args->args_arg == nullptr);
    REQUIRE(args->kwargs_arg == nullptr);
    REQUIRE(args->args[0]->name.value == "a");
    REQUIRE(args->args[1]->name.value == "b");

    auto* body = dynamic_cast<ScopeASTNode*>(function->body.get());
    REQUIRE(body != nullptr);
    REQUIRE(body->nodes.size() == 1);

    auto* ret = dynamic_cast<ReturnStatementASTNode*>(body->nodes[0].get());
    REQUIRE(ret != nullptr);

    auto* expr = dynamic_cast<BinaryOpASTNode*>(ret->expr.get());
    REQUIRE(expr != nullptr);
    REQUIRE(expr->op.type == TOKEN_TYPE::PLUS);

    auto* lhs = dynamic_cast<IdentifierASTNode*>(expr->LHS.get());
    auto* rhs = dynamic_cast<IdentifierASTNode*>(expr->RHS.get());

    REQUIRE(lhs != nullptr);
    REQUIRE(rhs != nullptr);
    REQUIRE(lhs->token.value == "a");
    REQUIRE(rhs->token.value == "b");
}

TEST_CASE("parser parses function declaration with args and kwargs", "[parser]")
{
    auto ast = parse_text("fn collect(a, *rest, **named) { return a; }");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* function = dynamic_cast<FunctionASTNode*>(root->nodes[0].get());
    REQUIRE(function != nullptr);

    auto* decl = dynamic_cast<FunctionDeclASTNode*>(function->decl.get());
    REQUIRE(decl != nullptr);

    auto* args = dynamic_cast<FunctionArgumentListASTNode*>(decl->args.get());
    REQUIRE(args != nullptr);

    REQUIRE(args->args.size() == 1);
    REQUIRE(args->args[0]->name.value == "a");

    REQUIRE(args->args_arg != nullptr);
    REQUIRE(args->args_arg->name.value == "rest");
    REQUIRE(args->args_arg->is_args);
    REQUIRE(!args->args_arg->is_kwargs);

    REQUIRE(args->kwargs_arg != nullptr);
    REQUIRE(args->kwargs_arg->name.value == "named");
    REQUIRE(args->kwargs_arg->is_kwargs);
    REQUIRE(!args->kwargs_arg->is_args);
}

TEST_CASE("parser parses if else statement", "[parser]")
{
    auto ast = parse_text("if true { return 1; } else { return 2; }");
    auto* root = require_root(ast);

    REQUIRE(root->nodes.size() == 1);

    auto* if_else = dynamic_cast<IfElseExpressionASTNode*>(root->nodes[0].get());
    REQUIRE(if_else != nullptr);

    auto* condition = dynamic_cast<BooleanASTNode*>(if_else->condition.get());
    REQUIRE(condition != nullptr);
    REQUIRE(condition->value);

    auto* true_scope = dynamic_cast<ScopeASTNode*>(if_else->true_scope.get());
    auto* false_scope = dynamic_cast<ScopeASTNode*>(if_else->false_scope.get());

    REQUIRE(true_scope != nullptr);
    REQUIRE(false_scope != nullptr);
    REQUIRE(true_scope->nodes.size() == 1);
    REQUIRE(false_scope->nodes.size() == 1);

    auto* true_return = dynamic_cast<ReturnStatementASTNode*>(true_scope->nodes[0].get());
    auto* false_return = dynamic_cast<ReturnStatementASTNode*>(false_scope->nodes[0].get());

    REQUIRE(true_return != nullptr);
    REQUIRE(false_return != nullptr);

    auto* true_value = dynamic_cast<IntegerASTNode*>(true_return->expr.get());
    auto* false_value = dynamic_cast<IntegerASTNode*>(false_return->expr.get());

    REQUIRE(true_value != nullptr);
    REQUIRE(false_value != nullptr);
    REQUIRE(true_value->value == 1);
    REQUIRE(false_value->value == 2);
}

TEST_CASE("parser throws syntax error", "[parser]")
{
  parse_text("call(1, 2");
  parse_text("call(1, 2),");
  parse_text("call(*args, **kwargs)");
  parse_text("call(*args, *kwargs)");
  parse_text("call(**args, **kwargs)");
}