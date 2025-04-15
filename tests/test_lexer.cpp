//
// Created by lenin on 13.11.2024.
//

#include <catch2/catch_test_macros.hpp>
#include "yapl/Lexer.hpp"
#include "yapl/Token.hpp"

TEST_CASE("Test empty text", "[lexer]")
{
    yapl::Lexer lexer{ "" };
    auto tokens = lexer.make_tokens();

    REQUIRE(tokens[0].type == yapl::TOKEN_TYPE::TT_EOF);
}
TEST_CASE("Test literals", "[lexer]")
{
    yapl::Lexer lexer{ R"(69 42.12 "string" 'another string' true false "string with special chars \n\\")" };
    auto tokens = lexer.make_tokens();

    REQUIRE(tokens[0].type == yapl::TOKEN_TYPE::INTEGER);
    REQUIRE(tokens[0].value == std::string("69"));

    REQUIRE(tokens[1].type == yapl::TOKEN_TYPE::FLOAT);
    REQUIRE(tokens[1].value == std::string("42.12"));

    REQUIRE(tokens[2].type == yapl::TOKEN_TYPE::STRING);
    REQUIRE(tokens[2].value == std::string("string"));

    REQUIRE(tokens[3].type == yapl::TOKEN_TYPE::STRING);
    REQUIRE(tokens[3].value == std::string("another string"));

    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::BOOL);
    REQUIRE(tokens[4].value == std::string("true"));

    REQUIRE(tokens[5].type == yapl::TOKEN_TYPE::BOOL);
    REQUIRE(tokens[5].value == std::string("false"));

    REQUIRE(tokens[6].type == yapl::TOKEN_TYPE::STRING);
    REQUIRE(tokens[6].value == std::string("string with special chars \n\\"));

    REQUIRE(tokens[7].type == yapl::TOKEN_TYPE::TT_EOF);
}

TEST_CASE("Test identifiers", "[lexer]")
{
    yapl::Lexer lexer{ "apple another_identifier with_numbers_42_and_underscores CamelCase" };
    auto tokens = lexer.make_tokens();


    REQUIRE(tokens[0].type == yapl::TOKEN_TYPE::IDENTIFIER);
    REQUIRE(tokens[0].value == std::string("apple"));

    REQUIRE(tokens[1].type == yapl::TOKEN_TYPE::IDENTIFIER);
    REQUIRE(tokens[1].value == std::string("another_identifier"));

    REQUIRE(tokens[2].type == yapl::TOKEN_TYPE::IDENTIFIER);
    REQUIRE(tokens[2].value == std::string("with_numbers_42_and_underscores"));

    REQUIRE(tokens[3].type == yapl::TOKEN_TYPE::IDENTIFIER);
    REQUIRE(tokens[3].value == std::string("CamelCase"));

    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::TT_EOF);
}

TEST_CASE("Test keywords", "[lexer]")
{
    yapl::Lexer lexer{ "if else for while fn var let const return" };
    auto tokens = lexer.make_tokens();

    REQUIRE(tokens[0].type == yapl::TOKEN_TYPE::IF);
    REQUIRE(tokens[1].type == yapl::TOKEN_TYPE::ELSE);
    REQUIRE(tokens[2].type == yapl::TOKEN_TYPE::FOR);
    REQUIRE(tokens[3].type == yapl::TOKEN_TYPE::WHILE);
    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::FN);
    REQUIRE(tokens[5].type == yapl::TOKEN_TYPE::VAR);
    REQUIRE(tokens[6].type == yapl::TOKEN_TYPE::LET);
    REQUIRE(tokens[7].type == yapl::TOKEN_TYPE::CONST);
    REQUIRE(tokens[8].type == yapl::TOKEN_TYPE::RETURN);

    REQUIRE(tokens[9].type == yapl::TOKEN_TYPE::TT_EOF);
}

TEST_CASE("Test single-line comment removal", "[lexer]")
{
    yapl::Lexer lexer{ "first_line;\nidentifier; // blah-blah-blah comment\nnext_line_identifier;" };
    auto tokens = lexer.make_tokens();

    REQUIRE(tokens[0].type == yapl::TOKEN_TYPE::IDENTIFIER);
    REQUIRE(tokens[0].value == std::string("first_line"));

    REQUIRE(tokens[1].type == yapl::TOKEN_TYPE::SEMICOLON);

    REQUIRE(tokens[2].type == yapl::TOKEN_TYPE::IDENTIFIER);
    REQUIRE(tokens[2].value == std::string("identifier"));

    REQUIRE(tokens[3].type == yapl::TOKEN_TYPE::SEMICOLON);

    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::IDENTIFIER);
    REQUIRE(tokens[4].value == std::string("next_line_identifier"));

    REQUIRE(tokens[5].type == yapl::TOKEN_TYPE::SEMICOLON);

    REQUIRE(tokens[6].type == yapl::TOKEN_TYPE::TT_EOF);
}

TEST_CASE("Test line and col numbers", "[lexer]")
{
    yapl::Lexer lexer{ "first_line_identifier\nsecond third //comment\n fourth" };
    auto tokens = lexer.make_tokens();

    REQUIRE(tokens[0].line == 1);
    REQUIRE(tokens[0].col_start == 1);
    REQUIRE(tokens[0].col_end == 21);

    REQUIRE(tokens[1].line == 2);
    REQUIRE(tokens[1].col_start == 1);
    REQUIRE(tokens[1].col_end == 6);

    REQUIRE(tokens[2].line == 2);
    REQUIRE(tokens[2].col_start == 8);
    REQUIRE(tokens[2].col_end == 12);

    REQUIRE(tokens[3].line == 3);
    REQUIRE(tokens[3].col_start == 2);
    REQUIRE(tokens[3].col_end == 7);
}

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