//
// Created by lenin on 13.11.2024.
//

#include <catch2/catch_test_macros.hpp>
#include "yapl/Lexer.hpp"
#include "yapl/Token.hpp"

#include <array>

using enum yapl::TOKEN_TYPE;

void check_token_types(std::string_view src,
                       std::span<const yapl::TOKEN_TYPE> expected)
{
    yapl::Lexer lex{src};
    auto toks = lex.make_tokens();

    REQUIRE(toks.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
        CHECK(toks[i].type == expected[i]);
}

void check_token_types_and_values(std::string_view src,
                       std::span<yapl::Token> expected)
{
    yapl::Lexer lex{src};
    auto toks = lex.make_tokens();

    REQUIRE(toks.size() == expected.size());
    for (std::size_t i = 0; i < expected.size(); ++i)
    {
        CHECK(toks[i].type == expected[i].type);
        CHECK(std::string(toks[i].value) == std::string(expected[i].value));
    }
}
TEST_CASE("Test empty text", "[lexer]")
{
    check_token_types("", std::array{ TT_EOF });
}

TEST_CASE("Test keywords and operators", "[lexer]")
{
    check_token_types("+ - * % / . , ! = == != < <= > >= => or and if else for fn var let const return while ;",
                      std::array{ PLUS, MINUS, TIMES, MOD, SLASH, PERIOD, COMMA,
                                          NOT, ASSIGN, EQ, NEQ, LT, LQ, GT, GQ, ARROW,
                                          OR, AND, IF, ELSE, FOR, FN, VAR, LET, CONST,
                                          RETURN, WHILE, SEMICOLON, TT_EOF });
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

    REQUIRE(tokens[7].type == yapl::TOKEN_TYPE::SEMICOLON);
    REQUIRE(tokens[8].type == yapl::TOKEN_TYPE::TT_EOF);
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

    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::SEMICOLON);
    REQUIRE(tokens[5].type == yapl::TOKEN_TYPE::TT_EOF);
}

TEST_CASE("Test keywords", "[lexer]")
{
    yapl::Lexer lexer{ "if else for while fn let const return" };
    auto tokens = lexer.make_tokens();

    REQUIRE(tokens[0].type == yapl::TOKEN_TYPE::IF);
    REQUIRE(tokens[1].type == yapl::TOKEN_TYPE::ELSE);
    REQUIRE(tokens[2].type == yapl::TOKEN_TYPE::FOR);
    REQUIRE(tokens[3].type == yapl::TOKEN_TYPE::WHILE);
    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::FN);
    REQUIRE(tokens[6].type == yapl::TOKEN_TYPE::LET);
    REQUIRE(tokens[7].type == yapl::TOKEN_TYPE::CONST);
    REQUIRE(tokens[8].type == yapl::TOKEN_TYPE::RETURN);

    REQUIRE(tokens[9].type == yapl::TOKEN_TYPE::SEMICOLON);
    REQUIRE(tokens[10].type == yapl::TOKEN_TYPE::TT_EOF);
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
    yapl::Lexer lexer{ "first_line_identifier;\nsecond; third; //comment\n fourth;" };
    auto tokens = lexer.make_tokens();

    REQUIRE(tokens[0].line == 1);
    REQUIRE(tokens[0].col_start == 1);
    REQUIRE(tokens[0].col_end == 21);

    REQUIRE(tokens[2].line == 2);
    REQUIRE(tokens[2].col_start == 1);
    REQUIRE(tokens[2].col_end == 6);

    REQUIRE(tokens[4].line == 2);
    REQUIRE(tokens[4].col_start == 9);
    REQUIRE(tokens[4].col_end == 13);

    REQUIRE(tokens[6].line == 3);
    REQUIRE(tokens[6].col_start == 2);
    REQUIRE(tokens[6].col_end == 7);
}

TEST_CASE("Test semicolon insertion", "[lexer]")
{
    SECTION("Simple expression")
        check_token_types("10",
                          std::array{INTEGER, SEMICOLON, TT_EOF});

    SECTION("Unfinished expression")
        check_token_types("10 +",
                          std::array{INTEGER, PLUS, TT_EOF});

    SECTION("Multi-line expression")
        check_token_types("6 +\n9",
                          std::array{INTEGER, PLUS, INTEGER, SEMICOLON, TT_EOF});

    SECTION("Unclosed scope")
        check_token_types("if a < b {",
                          std::array{IF, IDENTIFIER, LT, IDENTIFIER, LBRACK, TT_EOF});

    SECTION("If scope on another line")
        check_token_types("if a < b\n{}",
                          std::array{IF, IDENTIFIER, LT, IDENTIFIER, LBRACK, RBRACK, TT_EOF});

    SECTION("If scope on another line")
        check_token_types("if a < b\n{\nprint(penis)\n}",
                          std::array{IF, IDENTIFIER, LT, IDENTIFIER, LBRACK, IDENTIFIER, LPAREN, IDENTIFIER, RPAREN, SEMICOLON, RBRACK, TT_EOF});
}

TEST_CASE("Test basic lexer tokens", "[lexer]")
{
    yapl::Lexer lexer{ "69 * (400 + 20) / 48.2" };
    auto tokens = lexer.make_tokens();
    REQUIRE(tokens.size() == 11);
    REQUIRE((tokens[0].type == yapl::TOKEN_TYPE::INTEGER && strcmp(tokens[0].value, "69") == 0));
    REQUIRE(tokens[1].type == yapl::TOKEN_TYPE::TIMES);
    REQUIRE(tokens[2].type == yapl::TOKEN_TYPE::LPAREN);
    REQUIRE((tokens[3].type == yapl::TOKEN_TYPE::INTEGER && strcmp(tokens[3].value, "400") == 0));
    REQUIRE(tokens[4].type == yapl::TOKEN_TYPE::PLUS);
    REQUIRE((tokens[5].type == yapl::TOKEN_TYPE::INTEGER && strcmp(tokens[5].value, "20") == 0));
    REQUIRE(tokens[6].type == yapl::TOKEN_TYPE::RPAREN);
    REQUIRE(tokens[7].type == yapl::TOKEN_TYPE::SLASH);
    REQUIRE((tokens[8].type == yapl::TOKEN_TYPE::FLOAT && strcmp(tokens[8].value, "48.2") == 0));
    REQUIRE(tokens[9].type == yapl::TOKEN_TYPE::SEMICOLON);
    REQUIRE(tokens[10].type == yapl::TOKEN_TYPE::TT_EOF);
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