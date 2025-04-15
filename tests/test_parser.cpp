//
// Created by lenin on 16.04.2025.
//

#include <string>
#include <sstream>

#include <catch2/catch_test_macros.hpp>
#include "yapl/Lexer.hpp"
#include "yapl/Parser.hpp"


std::vector<std::string> get_lines_from_text(const std::string& text)
{
    std::vector<std::string> res;
    std::stringstream ss { text };
    while (!ss.eof())
    {
        std::string line;
        std::getline(ss, line);
        res.push_back(line);
    }
    return res;
}

TEST_CASE("Parsing basic expressions", "[parser]")
{
    std::string text = "4 + 2 * 6 - (1+2) / 8;";
    Lexer lexer{ text };
    Parser parser { lexer.make_tokens(), "<test_case>", get_lines_from_text(text)  };
    auto ast = parser.parse_root();
    auto root = dynamic_cast<RootASTNode*>(ast.get());
    REQUIRE(root != nullptr);

    auto minus = dynamic_cast<BinaryOpASTNode*>(root->nodes[0].get());
    REQUIRE(minus != nullptr);
    REQUIRE(minus->op.type == yapl::TOKEN_TYPE::MINUS);
    auto minus_LHS = dynamic_cast<BinaryOpASTNode*>(minus->LHS.get());
    auto minus_RHS = dynamic_cast<BinaryOpASTNode*>(minus->RHS.get());

    REQUIRE(minus_LHS != nullptr);
    REQUIRE(minus_LHS->op.type == yapl::TOKEN_TYPE::PLUS);
    auto plus1_LHS = dynamic_cast<LiteralASTNode*>(minus_LHS->LHS.get());
    auto plus1_RHS = dynamic_cast<BinaryOpASTNode*>(minus_LHS->RHS.get());

    REQUIRE(plus1_LHS->token.value == std::string("4"));

    REQUIRE(plus1_RHS != nullptr);
    REQUIRE(plus1_RHS->op.type == yapl::TOKEN_TYPE::TIMES);
    auto times_LHS = dynamic_cast<LiteralASTNode*>(plus1_RHS->LHS.get());
    auto times_RHS = dynamic_cast<LiteralASTNode*>(plus1_RHS->RHS.get());

    REQUIRE(times_LHS->token.value == std::string("2"));
    REQUIRE(times_RHS->token.value == std::string("6"));

    REQUIRE(minus_RHS != nullptr);
    REQUIRE(minus_RHS->op.type == yapl::TOKEN_TYPE::SLASH);
    auto slash_LHS = dynamic_cast<BinaryOpASTNode*>(minus_RHS->LHS.get());
    auto slash_RHS = dynamic_cast<LiteralASTNode*>(minus_RHS->RHS.get());

    REQUIRE(slash_LHS != nullptr);
    REQUIRE(slash_LHS->op.type == yapl::TOKEN_TYPE::PLUS);
    auto plus2_LHS = dynamic_cast<LiteralASTNode*>(slash_LHS->LHS.get());
    auto plus2_RHS = dynamic_cast<LiteralASTNode*>(slash_LHS->RHS.get());

    REQUIRE(plus2_LHS->token.value == std::string("1"));
    REQUIRE(plus2_RHS->token.value == std::string("2"));

    REQUIRE(slash_RHS->token.value == std::string("8"));


//    REQUIRE(ast.to_canon_string() == "(root (bin_op - (bin_op + 4 (bin_op * 2 6)) (bin_op / (bin_op + 1 2) 8)))");



}