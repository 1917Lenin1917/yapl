//
// Created by lenin on 13.11.2024.
//

#include <catch2/catch_test_macros.hpp>
#include "../src/Lexer.hpp"
#include "../src/Token.hpp"

TEST_CASE("Test basic lexer tokens", "[lexer]")
{
    yapl::Lexer lexer{ "69 * (400 + 20) / 48.2" };
    auto tokens = lexer.make_tokens();
    REQUIRE(tokens.size() == 9);
    REQUIRE((tokens[0].type == yapl::TOKEN_TYPE::INTEGER && strcmp(tokens[0].value, "69") == 0));
    REQUIRE(tokens[1].type == yapl::TOKEN_TYPE::MUL);
    REQUIRE(tokens[2].type == yapl::TOKEN_TYPE::LPAREN);
    REQUIRE((tokens[3].type == yapl::TOKEN_TYPE::INTEGER && strcmp(tokens[3].value, "400") == 0));
    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::PLUS);
    REQUIRE((tokens[5].type == yapl::TOKEN_TYPE::INTEGER && strcmp(tokens[5].value, "20") == 0));
    REQUIRE(tokens[6].type == yapl::TOKEN_TYPE::RPAREN);
    REQUIRE(tokens[7].type == yapl::TOKEN_TYPE::DIV);
    REQUIRE((tokens[8].type == yapl::TOKEN_TYPE::FLOAT && strcmp(tokens[8].value, "48.2") == 0));
}