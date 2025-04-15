//
// Created by lenin on 13.11.2024.
//

#include <catch2/catch_test_macros.hpp>
#include "yapl/Lexer.hpp"
#include "yapl/Token.hpp"

TEST_CASE("Test basic lexer tokens", "[lexer]")
{
    yapl::Lexer lexer{ "69 * (400 + 20) / 48.2" };
    auto tokens = lexer.make_tokens();
    REQUIRE(tokens.size() == 10);
    REQUIRE((tokens[0].type == yapl::TOKEN_TYPE::INTEGER && strcmp(tokens[0].value, "69") == 0));
    REQUIRE(tokens[1].type == yapl::TOKEN_TYPE::TIMES);
    REQUIRE(tokens[2].type == yapl::TOKEN_TYPE::LPAREN);
    REQUIRE((tokens[3].type == yapl::TOKEN_TYPE::INTEGER && strcmp(tokens[3].value, "400") == 0));
    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::PLUS);
    REQUIRE((tokens[5].type == yapl::TOKEN_TYPE::INTEGER && strcmp(tokens[5].value, "20") == 0));
    REQUIRE(tokens[6].type == yapl::TOKEN_TYPE::RPAREN);
    REQUIRE(tokens[7].type == yapl::TOKEN_TYPE::SLASH);
    REQUIRE((tokens[8].type == yapl::TOKEN_TYPE::FLOAT && strcmp(tokens[8].value, "48.2") == 0));
    REQUIRE(tokens[9].type == yapl::TOKEN_TYPE::TT_EOF);
}

TEST_CASE("Test basic function declaration", "[lexer]")
{
    yapl::Lexer lexer{ "fn foo() { return 123; }" };
    auto tokens = lexer.make_tokens();
    REQUIRE(tokens.size() == 10);
    REQUIRE(tokens[0].type == yapl::TOKEN_TYPE::FN);

    REQUIRE(tokens[1].type == yapl::TOKEN_TYPE::IDENTIFIER);
    REQUIRE(strcmp(tokens[1].value, "foo") == 0);

    REQUIRE(tokens[2].type == yapl::TOKEN_TYPE::LPAREN);
    REQUIRE(tokens[3].type == yapl::TOKEN_TYPE::RPAREN);

    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::LBRACK);
    REQUIRE(tokens[5].type == yapl::TOKEN_TYPE::RETURN);
    REQUIRE(tokens[6].type == yapl::TOKEN_TYPE::INTEGER);
    REQUIRE(strcmp(tokens[6].value, "123") == 0);
    REQUIRE(tokens[7].type == yapl::TOKEN_TYPE::SEMICOLON);
    REQUIRE(tokens[8].type == yapl::TOKEN_TYPE::RBRACK);

    REQUIRE(tokens[9].type == yapl::TOKEN_TYPE::TT_EOF);
}