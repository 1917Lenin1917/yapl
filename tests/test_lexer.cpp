#include <array>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include "yapl/Lexer.hpp"
#include "yapl/Token.hpp"

using enum yapl::TOKEN_TYPE;

namespace {

auto tokenize(std::string_view source) -> std::vector<yapl::Token>
{
    yapl::Lexer lexer{source};
    return lexer.Tokenize();
}

void check_token_types(std::string_view source, std::span<const yapl::TOKEN_TYPE> expectedTypes)
{
    const auto tokens = tokenize(source);

    REQUIRE(tokens.size() == expectedTypes.size());
    for (std::size_t i = 0; i < expectedTypes.size(); ++i)
        CHECK(tokens[i].type == expectedTypes[i]);
}

void check_token_types_and_values(std::string_view source, std::span<const yapl::Token> expectedTokens)
{
    const auto tokens = tokenize(source);

    REQUIRE(tokens.size() == expectedTokens.size());
    for (std::size_t i = 0; i < expectedTokens.size(); ++i)
    {
        CHECK(tokens[i].type == expectedTokens[i].type);
        CHECK(std::string(tokens[i].value) == std::string(expectedTokens[i].value));
    }
}

} // namespace

TEST_CASE("Lexer tokenizes empty input", "[lexer]")
{
    check_token_types("", std::array{TT_EOF});
}

TEST_CASE("Lexer tokenizes whitespace-only input", "[lexer]")
{
    check_token_types(" \t\r   \t", std::array{TT_EOF});
}

TEST_CASE("Lexer tokenizes all current keywords", "[lexer]")
{
    check_token_types(
        "or and if else for while class fn let const return import export from",
        std::array{
            OR, AND, IF, ELSE, FOR, WHILE, CLASS, FN, LET, CONST, RETURN, IMPORT, EXPORT, FROM, SEMICOLON, TT_EOF
        }
    );
}

TEST_CASE("Lexer tokenizes identifiers and booleans", "[lexer]")
{
    const std::array expected{
        yapl::Token{ .type = IDENTIFIER, .value = "apple" },
        yapl::Token{ .type = IDENTIFIER, .value = "_hidden" },
        yapl::Token{ .type = IDENTIFIER, .value = "with_numbers_42" },
        yapl::Token{ .type = IDENTIFIER, .value = "CamelCase" },
        yapl::Token{ .type = BOOL, .value = "true" },
        yapl::Token{ .type = BOOL, .value = "false" },
        yapl::Token{ .type = SEMICOLON },
        yapl::Token{ .type = TT_EOF },
    };

    check_token_types_and_values("apple _hidden with_numbers_42 CamelCase true false", expected);
}

TEST_CASE("Lexer tokenizes punctuation and bracket tokens", "[lexer]")
{
    check_token_types(
        "( ) { } [ ] ; : ,",
        std::array{
            LPAREN, RPAREN,
            LBRACK, RBRACK,
            LSQBRACK, RSQBRACK,
            SEMICOLON, COLON, COMMA,
            TT_EOF
        }
    );
}

TEST_CASE("Lexer tokenizes single-character operators", "[lexer]")
{
    check_token_types(
        "+ - * % / ! < > =",
        std::array{
            PLUS, MINUS, TIMES, MOD, SLASH, NOT, LT, GT, ASSIGN, SEMICOLON, TT_EOF
        }
    );
}

TEST_CASE("Lexer tokenizes equals-form operators", "[lexer]")
{
    check_token_types(
        "+= -= *= %= /= != <= >= ==",
        std::array{
            PLUSEQ, MINUSEQ, TIMESEQ, MODEQ, SLASHEQ, NEQ, LQ, GQ, EQ, SEMICOLON, TT_EOF
        }
    );
}

TEST_CASE("Lexer tokenizes integers and floats", "[lexer]")
{
    const std::array expected{
        yapl::Token{ .type = INTEGER, .value = "0" },
        yapl::Token{ .type = INTEGER, .value = "69" },
        yapl::Token{ .type = FLOAT, .value = "42.12" },
        yapl::Token{ .type = FLOAT, .value = "1." },
        yapl::Token{ .type = SEMICOLON },
        yapl::Token{ .type = TT_EOF },
    };

    check_token_types_and_values("0 69 42.12 1.", expected);
}

TEST_CASE("Lexer tokenizes quoted strings", "[lexer]")
{
    const std::array expected{
        yapl::Token{ .type = STRING, .value = "hello" },
        yapl::Token{ .type = STRING, .value = "world" },
        yapl::Token{ .type = SEMICOLON },
        yapl::Token{ .type = TT_EOF },
    };

    check_token_types_and_values(R"("hello" 'world')", expected);
}

TEST_CASE("Lexer tokenizes string escapes", "[lexer]")
{
    const std::array expected{
        yapl::Token{ .type = STRING, .value = "line1\nline2\\" },
        yapl::Token{ .type = SEMICOLON },
        yapl::Token{ .type = TT_EOF },
    };

    check_token_types_and_values(R"("line1\nline2\\")", expected);
}

TEST_CASE("Lexer tokenizes format strings as FSTRING", "[lexer]")
{
    const std::array expected{
        yapl::Token{ .type = FSTRING, .value = "hello {name}" },
        yapl::Token{ .type = SEMICOLON },
        yapl::Token{ .type = TT_EOF },
    };

    check_token_types_and_values("`hello {name}`", expected);
}

TEST_CASE("Lexer skips single-line comments", "[lexer]")
{
    const std::array expected{
        IDENTIFIER, SEMICOLON,
        IDENTIFIER, SEMICOLON,
        IDENTIFIER, SEMICOLON,
        TT_EOF
    };

    check_token_types(
        "first;\nsecond; // comment here\nthird",
        expected
    );
}

TEST_CASE("Lexer inserts semicolon at end of simple expression", "[lexer]")
{
    check_token_types("10", std::array{INTEGER, SEMICOLON, TT_EOF});
}

TEST_CASE("Lexer does not insert semicolon after unfinished expression", "[lexer]")
{
    check_token_types("10 +", std::array{INTEGER, PLUS, TT_EOF});
}

TEST_CASE("Lexer does not insert semicolon in parenthesized multiline expression", "[lexer]")
{
    check_token_types(
        "(1 +\n2)",
        std::array{LPAREN, INTEGER, PLUS, INTEGER, RPAREN, SEMICOLON, TT_EOF}
    );
}

TEST_CASE("Lexer does not insert semicolon in square-bracket multiline expression", "[lexer]")
{
    check_token_types(
        "[1,\n2]",
        std::array{LSQBRACK, INTEGER, COMMA, INTEGER, RSQBRACK, SEMICOLON, TT_EOF}
    );
}

TEST_CASE("Lexer does not insert semicolon before block opener on next line", "[lexer]")
{
    check_token_types(
        "if condition\n{}",
        std::array{IF, IDENTIFIER, LBRACK, RBRACK, TT_EOF}
    );
}

TEST_CASE("Lexer does not insert semicolon before property access on next line", "[lexer]")
{
    check_token_types(
        "foo\n.bar",
        std::array{IDENTIFIER, PERIOD, IDENTIFIER, SEMICOLON, TT_EOF}
    );
}

TEST_CASE("Lexer does not insert semicolon before assignment on next line", "[lexer]")
{
    check_token_types(
        "foo\n= 1",
        std::array{IDENTIFIER, ASSIGN, INTEGER, SEMICOLON, TT_EOF}
    );
}

TEST_CASE("Lexer does not insert semicolons inside import list braces", "[lexer]")
{
    check_token_types(
        "import {\na,\nb\n} from \"mod\"",
        std::array{
            IMPORT, LBRACK, IDENTIFIER, COMMA, IDENTIFIER, RBRACK, FROM, STRING, SEMICOLON, TT_EOF
        }
    );
}

TEST_CASE("Lexer does not insert semicolons inside export list braces", "[lexer]")
{
    check_token_types(
        "export {\na,\nb\n}",
        std::array{
            EXPORT, LBRACK, IDENTIFIER, COMMA, IDENTIFIER, RBRACK, TT_EOF
        }
    );
}

TEST_CASE("Lexer clears pending export for declaration export forms", "[lexer]")
{
    check_token_types(
        "export fn foo() {}",
        std::array{
            EXPORT, FN, IDENTIFIER, LPAREN, RPAREN, LBRACK, RBRACK, TT_EOF
        }
    );
}

TEST_CASE("Lexer does not auto-insert semicolon after block closing brace", "[lexer]")
{
    check_token_types(
        "if cond {\nvalue\n}\nnext",
        std::array{
            IF, IDENTIFIER, LBRACK, IDENTIFIER, SEMICOLON, RBRACK, IDENTIFIER, SEMICOLON, TT_EOF
        }
    );
}

TEST_CASE("Lexer auto-inserts semicolon after object-expression closing brace", "[lexer]")
{
    check_token_types(
        "return { answer: 42 }\nnext",
        std::array{
            RETURN, LBRACK, IDENTIFIER, COLON, INTEGER, RBRACK, SEMICOLON, IDENTIFIER, SEMICOLON, TT_EOF
        }
    );
}

TEST_CASE("Lexer tokenizes a representative function", "[lexer]")
{
    check_token_types(
        "fn foo() { return 123; }",
        std::array{
            FN, IDENTIFIER, LPAREN, RPAREN, LBRACK, RETURN, INTEGER, SEMICOLON, RBRACK, TT_EOF
        }
    );
}

TEST_CASE("Lexer tokenizes a representative expression", "[lexer]")
{
    const auto tokens = tokenize("69 * (400 + 20) / 48.2");

    REQUIRE(tokens.size() == 11);

    REQUIRE(tokens[0].type == INTEGER);
    REQUIRE(tokens[0].value == "69");

    REQUIRE(tokens[1].type == TIMES);
    REQUIRE(tokens[2].type == LPAREN);

    REQUIRE(tokens[3].type == INTEGER);
    REQUIRE(tokens[3].value == "400");

    REQUIRE(tokens[4].type == PLUS);

    REQUIRE(tokens[5].type == INTEGER);
    REQUIRE(tokens[5].value == "20");

    REQUIRE(tokens[6].type == RPAREN);
    REQUIRE(tokens[7].type == SLASH);

    REQUIRE(tokens[8].type == FLOAT);
    REQUIRE(tokens[8].value == "48.2");

    REQUIRE(tokens[9].type == SEMICOLON);
    REQUIRE(tokens[10].type == TT_EOF);
}

TEST_CASE("Lexer stores identifier ranges", "[lexer]")
{
    const auto tokens = tokenize("alpha beta");

    REQUIRE(tokens.size() == 4);

    REQUIRE(tokens[0].type == IDENTIFIER);
    CHECK(tokens[0].range.start.line == 0);
    CHECK(tokens[0].range.start.character == 0);
    CHECK(tokens[0].range.end.line == 0);
    CHECK(tokens[0].range.end.character == 4);

    REQUIRE(tokens[1].type == IDENTIFIER);
    CHECK(tokens[1].range.start.line == 0);
    CHECK(tokens[1].range.start.character == 6);
    CHECK(tokens[1].range.end.line == 0);
    CHECK(tokens[1].range.end.character == 9);
}

TEST_CASE("Lexer stores multiline ranges", "[lexer]")
{
    const auto tokens = tokenize("first\nsecond");

    REQUIRE(tokens.size() == 5);

    REQUIRE(tokens[0].type == IDENTIFIER);
    CHECK(tokens[0].range.start.line == 0);
    CHECK(tokens[0].range.start.character == 0);
    CHECK(tokens[0].range.end.line == 0);
    CHECK(tokens[0].range.end.character == 4);

    REQUIRE(tokens[2].type == IDENTIFIER);
    CHECK(tokens[2].range.start.line == 1);
    CHECK(tokens[2].range.start.character == 0);
    CHECK(tokens[2].range.end.line == 1);
    CHECK(tokens[2].range.end.character == 5);
}

TEST_CASE("Lexer tokenizes period without creating LPAREN", "[lexer]")
{
    const auto tokens = tokenize("foo.bar");

    REQUIRE(tokens.size() == 5);
    CHECK(tokens[0].type == IDENTIFIER);
    CHECK(tokens[1].type == PERIOD);
    CHECK(tokens[2].type == IDENTIFIER);
    CHECK(tokens[3].type == SEMICOLON);
    CHECK(tokens[4].type == TT_EOF);
}
