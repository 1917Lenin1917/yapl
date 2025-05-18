//
// Created by lenin on 13.11.2024.
//

#include "yapl/Lexer.hpp"

#include <unordered_map>

namespace yapl {

void Lexer::check_insert_semicolon(std::vector<Token>& tokens)
{
    // At every newline we try to auto-insert a semicolon, since parser grammar already requires them
    // We should check for
    // 1) now parens and brackets are open
    // 2) last token on the line is not an operator like + - * / % && || ( [ or and
    // TODO: add logical operators like && || or and
    // 3) next line doesn't start with . or =
    char next_non_white_space = '\0';
    for (auto i = m_pos+1; i < m_text.size(); i++)
    {
        // TODO: look out for comments
        next_non_white_space = m_text[i];
        if (next_non_white_space != '\0' && next_non_white_space != '\r' && next_non_white_space != '\t' && next_non_white_space != ' ' && next_non_white_space != '\n')
            break;
    }

    if (!tokens.empty())
    {
        const auto& last_token = tokens[tokens.size()-1];
        if (last_token.type != TOKEN_TYPE::PLUS &&
            last_token.type != TOKEN_TYPE::MINUS &&
            last_token.type != TOKEN_TYPE::TIMES &&
            last_token.type != TOKEN_TYPE::MOD &&
            last_token.type != TOKEN_TYPE::SLASH &&
            last_token.type != TOKEN_TYPE::PERIOD &&
            last_token.type != TOKEN_TYPE::NOT &&
            last_token.type != TOKEN_TYPE::LT &&
            last_token.type != TOKEN_TYPE::GT &&
            last_token.type != TOKEN_TYPE::LPAREN &&
            last_token.type != TOKEN_TYPE::LBRACK &&
            last_token.type != TOKEN_TYPE::LSQBRACK &&
            last_token.type != TOKEN_TYPE::RBRACK &&
            last_token.type != TOKEN_TYPE::SEMICOLON &&
            next_non_white_space != '{' &&
            next_non_white_space != '.'
//            (paren_depth == 0 &&
//            brace_depth == 0 &&
//            sq_br_depth == 0
            // TODO: add nextline checking
            // TODO: change this when return { something }; statements are implemented. For now we just dont insert semicolons for }
            // paren checks should be tied to the line maybe?
                )
        {
            // this may be wrong on lines with comments
            tokens.emplace_back(TOKEN_TYPE::SEMICOLON, nullptr, current_line, current_col_pos+1, current_col_pos+1);
        }
    }

}

std::vector<Token> Lexer::make_tokens()
{
    std::vector<Token> tokens;
    const size_t text_len = m_text.length();

    while (m_pos != text_len)
    {
        m_pos += 1;
        current_col_pos += 1;
        if (m_pos >= text_len)
        {
            check_insert_semicolon(tokens);
            if (tokens.empty() || tokens[tokens.size()-1].type != TOKEN_TYPE::TT_EOF)
                tokens.emplace_back(TOKEN_TYPE::TT_EOF);
            return tokens;
        }
        switch (const char c = m_text[m_pos])
        {
            case '\0':
            {
                check_insert_semicolon(tokens);
                tokens.emplace_back(TOKEN_TYPE::TT_EOF);
                break;
            }
            case '\n':
            {
                check_insert_semicolon(tokens);
                current_line += 1;
                current_col_pos = 0;
                break;
            }
            case '\t':
            case '\r':
            case ' ': { continue; }
            case '+':
            {
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::PLUSEQ, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::PLUS, nullptr, current_line, current_col_pos, current_col_pos);
                break;
            }
            case '-':
            {
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::MINUSEQ, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::MINUS, nullptr, current_line, current_col_pos, current_col_pos);
                break;
            }
            case '*':
            {
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::TIMESEQ, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::TIMES, nullptr, current_line, current_col_pos, current_col_pos);
                break;
            }
            case '%':
            {
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::MODEQ, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::MOD, nullptr, current_line, current_col_pos, current_col_pos);
                break;
            }
            case '/':
            {
                if (m_pos + 1 != text_len && m_text[m_pos+1] == '/')
                {
                    m_pos += 2; // eat //
                    while (m_pos != text_len && m_text[m_pos] != '\n')
                    {
                      m_pos++; // eat everything until \n or EOF
                    }
                    current_line += 1;
                    current_col_pos = 0;
                    break;
                }
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::SLASHEQ, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::SLASH, nullptr, current_line, current_col_pos, current_col_pos);
                break;
            }
            case '.': { tokens.emplace_back(TOKEN_TYPE::PERIOD, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case '!':
            {
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::NEQ, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::NOT, nullptr, current_line, current_col_pos, current_col_pos);
                break;
            }
            case '<':
            {
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::LQ, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::LT, nullptr, current_line, current_col_pos, current_col_pos);
                break;
            }
            case '>':
            {
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::GQ, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::GT, nullptr, current_line, current_col_pos, current_col_pos);
                break;
            }
            case '(': { paren_depth++; tokens.emplace_back(TOKEN_TYPE::LPAREN, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case ')': { paren_depth--; tokens.emplace_back(TOKEN_TYPE::RPAREN, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case '{': { brace_depth++; tokens.emplace_back(TOKEN_TYPE::LBRACK, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case '}': { brace_depth--; tokens.emplace_back(TOKEN_TYPE::RBRACK, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case '[': { sq_br_depth++; tokens.emplace_back(TOKEN_TYPE::LSQBRACK, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case ']': { sq_br_depth--; tokens.emplace_back(TOKEN_TYPE::RSQBRACK, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case ';': { tokens.emplace_back(TOKEN_TYPE::SEMICOLON, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case ':': { tokens.emplace_back(TOKEN_TYPE::COLON, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case ',': { tokens.emplace_back(TOKEN_TYPE::COMMA, nullptr, current_line, current_col_pos, current_col_pos); break; }
            case '\'':
            case '\"': { tokens.push_back(make_string()); break; }
            case '`': { tokens.push_back(make_format_string()); break; }
            case '=':
            {
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::EQ, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '>')
                {
                    tokens.emplace_back(TOKEN_TYPE::ARROW, nullptr, current_line, current_col_pos, current_col_pos);
                    m_pos++;
                    current_col_pos += 1;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::ASSIGN, nullptr, current_line, current_col_pos, current_col_pos+1);
                break;
            }
            default:
            {
                if (is_numeric(c))
                {
                    tokens.push_back(make_number());
                    break;
                }
                if (is_letter(c))
                {
                    tokens.push_back(make_identifier_or_keyword());
                }
            };
        }
    }
    return tokens;
}

Token Lexer::make_number()
{
    const size_t text_len = m_text.length();
    const int start_col_pos = current_col_pos;
    size_t start = m_pos;
    bool is_float = false;
    while (m_pos != text_len)
    {
        m_pos += 1;
        current_col_pos += 1;
        const char c = m_text[m_pos];
        if (!(c >= '0' && c <= '9' || c == '.'))
        {
            m_pos--;
            current_col_pos -= 1;
            break;
        }
        if (c == '.')
        {
            if (is_float)
            {
                m_pos--;
                current_col_pos -= 1;
                break;
            }
            is_float = true;
        }
    }
    char* value = new char[m_pos - start + 2];
    memcpy(value, m_text.data()+start, m_pos-start+1);
    value[m_pos - start + 1] = '\0';
    return { is_float ? TOKEN_TYPE::FLOAT : TOKEN_TYPE::INTEGER, value, current_line, (int)start_col_pos, (int)(current_col_pos) };
}

Token Lexer::make_string()
{
    const size_t text_len = m_text.length();
    const int start_col_pos = current_col_pos;
    size_t start = m_pos;
    while (m_pos != text_len)
    {
        m_pos += 1;
        current_col_pos += 1;
        char c = m_text[m_pos];
        if (c == m_text[start])
        {
            break;
        }
    }

    char* value = new char[m_pos - start + 2 - 2];
    memcpy(value, m_text.data()+start+1, m_pos-start+1-2);
    value[m_pos - start + 1-2] = '\0';
    size_t len = m_pos - start + 1 - 2;
    for (size_t i = 0; i < len; i++)
    {
        if (value[i] == '\\')
        {
            i++;
            switch (value[i])
            {
                case 'n':
                {
                    value[i-1] = '\n';
                    for (size_t j = i+1; j < len; j++)
                    {
                        value[j-1] = value[j];
                    }
                    len--;
                    i--;
                    break;
                }
                case '\\':
                {
                    value[i-1] = '\\';
                    for (size_t j = i+1; j < len; j++)
                    {
                        value[j-1] = value[j];
                    }
                    len--;
                    i--;
                    break;
                }
                default: {}
            }
        }
    }
    value[len] = '\0';
    return { m_text[m_pos] == '`' ? TOKEN_TYPE::FSTRING : TOKEN_TYPE::STRING, value, current_line, (int)start_col_pos, (int)(current_col_pos) };
}

Token Lexer::make_format_string() // TODO: rewrite to handle {}
{
    return make_string();
}

Token Lexer::make_identifier_or_keyword()
{
    const size_t text_len = m_text.length();
    size_t start = m_pos;
    const int start_col_pos = current_col_pos;
    while (m_pos != text_len)
    {
        m_pos += 1;
        current_col_pos += 1;
        char c = m_text[m_pos];
        if (!(is_letter(c) || is_numeric(c) || c == '_'))
        {
            m_pos--;
            current_col_pos -= 1;
            break;
        }
    }
    static const std::unordered_map<std::string_view, TOKEN_TYPE> kKeywordTable{
        { "or",     TOKEN_TYPE::OR },
        { "and",    TOKEN_TYPE::AND },
        { "if",     TOKEN_TYPE::IF },
        { "else",   TOKEN_TYPE::ELSE },
        { "for",    TOKEN_TYPE::FOR },
        { "while",  TOKEN_TYPE::WHILE },
        { "class",  TOKEN_TYPE::CLASS },
        { "fn",     TOKEN_TYPE::FN },
        { "var",    TOKEN_TYPE::VAR },
        { "let",    TOKEN_TYPE::LET },
        { "const",  TOKEN_TYPE::CONST },
        { "return", TOKEN_TYPE::RETURN }
    };

    const auto tk = std::string_view{ m_text.data() + start, m_pos - start + 1 };
    if (const auto it = kKeywordTable.find(tk); it != kKeywordTable.end()) {
        return { it->second, nullptr, current_line,start_col_pos,current_col_pos };
    }
    if (tk == "true" || tk == "false")
    {
        char* value = new char[m_pos - start + 2];
        memcpy(value, m_text.data()+start, m_pos-start+1);
        value[m_pos - start + 1] = '\0';
        return { TOKEN_TYPE::BOOL, value, current_line, (int)start_col_pos, (int)current_col_pos };
    }
    char* value = new char[m_pos - start + 2];
    memcpy(value, m_text.data()+start, m_pos-start+1);
    value[m_pos - start + 1] = '\0';
    return { TOKEN_TYPE::IDENTIFIER, value, current_line, (int)start_col_pos, (int)current_col_pos };
}


bool is_numeric(char c)
{
    return c >= '0' && c <= '9';
}

bool is_letter(char c)
{
    return c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z';
}

}
