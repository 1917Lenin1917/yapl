//
// Created by lenin on 13.11.2024.
//

#include "Lexer.hpp"

namespace yapl {

std::vector<Token> Lexer::make_tokens()
{
    std::vector<Token> tokens;
    const size_t text_len = m_text.length();

    while (m_pos != text_len)
    {
        m_pos += 1;
        if (m_pos == text_len) { return tokens; }
        switch (const char c = m_text[m_pos])
        {
            case '\0': { tokens.emplace_back(TOKEN_TYPE::TT_EOF); break; }
            case '\t':
            case '\n':
            case '\r':
            case ' ': { continue; }
            case '+': { tokens.emplace_back(TOKEN_TYPE::PLUS); break; }
            case '-': { tokens.emplace_back(TOKEN_TYPE::MINUS); break; }
            case '*': { tokens.emplace_back(TOKEN_TYPE::TIMES); break; }
            case '/':
            {
                if (m_pos + 1 != text_len && m_text[m_pos+1] == '/')
                {
                    m_pos += 2; // eat //
                    while (m_pos != text_len && m_text[m_pos] != '\n')
                    {
                      m_pos++; // eat everything until \n or EOF
                    }
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::SLASH);
                break;
            }
            case '.': { tokens.emplace_back(TOKEN_TYPE::PERIOD); break; }
            case '!': { tokens.emplace_back(TOKEN_TYPE::NOT); break; }
            case '<': { tokens.emplace_back(TOKEN_TYPE::LT); break; } // TODO: add <=
            case '>': { tokens.emplace_back(TOKEN_TYPE::GT); break; } // TODO: add >=
            case '(': { tokens.emplace_back(TOKEN_TYPE::LPAREN); break; }
            case ')': { tokens.emplace_back(TOKEN_TYPE::RPAREN); break; }
            case '{': { tokens.emplace_back(TOKEN_TYPE::LBRACK); break; }
            case '}': { tokens.emplace_back(TOKEN_TYPE::RBRACK); break; }
            case '[': { tokens.emplace_back(TOKEN_TYPE::LSQBRACK); break; }
            case ']': { tokens.emplace_back(TOKEN_TYPE::RSQBRACK); break; }
            case ';': { tokens.emplace_back(TOKEN_TYPE::SEMICOLON); break; }
            case ':': { tokens.emplace_back(TOKEN_TYPE::COLON); break; }
            case ',': { tokens.emplace_back(TOKEN_TYPE::COMMA); break; }
            case '\'':
            case '\"': { tokens.push_back(make_string()); break; }
            case '`': { tokens.push_back(make_format_string()); break; }
            case '=':
            {
                if (m_pos+1 < m_text.length() && m_text[m_pos+1] == '=')
                {
                    tokens.emplace_back(TOKEN_TYPE::EQ);
                    m_pos++;
                    break;
                }
                tokens.emplace_back(TOKEN_TYPE::ASSIGN);
                break;
            } // TODO: add =>
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
    size_t start = m_pos;
    bool is_float = false;
    while (m_pos != text_len)
    {
        m_pos += 1;
        const char c = m_text[m_pos];
        if (!(c >= '0' && c <= '9' || c == '.'))
        {
            m_pos--;
            break;
        }
        if (c == '.')
        {
            if (is_float)
            {
                m_pos--;
                break;
            }
            is_float = true;
        }
    }
    char* value = new char[m_pos - start + 1];
    memcpy(value, m_text.data()+start, m_pos-start+1);
    value[m_pos - start + 1] = '\0';
    return { is_float ? TOKEN_TYPE::FLOAT : TOKEN_TYPE::INTEGER, value };
}

Token Lexer::make_string()
{
    const size_t text_len = m_text.length();
    size_t start = m_pos;
    while (m_pos != text_len)
    {
        m_pos += 1;
        char c = m_text[m_pos];
        if (c == m_text[start])
        {
            break;
        }
    }

    char* value = new char[m_pos - start + 1 - 2];
    memcpy(value, m_text.data()+start+1, m_pos-start+1-2);
    value[m_pos - start + 1-2] = '\0';
    return { m_text[m_pos] == '`' ? TOKEN_TYPE::FSTRING : TOKEN_TYPE::STRING, value };
}

Token Lexer::make_format_string() // TODO: rewrite to handle {}
{
    return make_string();
}

Token Lexer::make_identifier_or_keyword()
{
    const size_t text_len = m_text.length();
    size_t start = m_pos;
    while (m_pos != text_len)
    {
        m_pos += 1;
        char c = m_text[m_pos];
        if (!(is_letter(c) || is_numeric(c) || c == '_'))
        {
            m_pos--;
            break;
        }
    }
    const auto tk = std::string_view{ m_text.data() + start, m_pos - start + 1 };
    if (tk == "if")
        return { TOKEN_TYPE::IF };
    if (tk == "else")
        return { TOKEN_TYPE::ELSE };
    if (tk == "for")
        return { TOKEN_TYPE::FOR };
    if (tk == "while")
        return { TOKEN_TYPE::WHILE };
    if (tk == "fn")
        return { TOKEN_TYPE::FN };
    if (tk == "var")
        return { TOKEN_TYPE::VAR };
    if (tk == "let")
        return { TOKEN_TYPE::LET };
    if (tk == "const")
        return { TOKEN_TYPE::CONST };
    if (tk == "return")
        return { TOKEN_TYPE::RETURN };
    if (tk == "true" || tk == "false")
    {
        char* value = new char[m_pos - start + 1];
        memcpy(value, m_text.data()+start, m_pos-start+1);
        value[m_pos - start + 1] = '\0';
        return { TOKEN_TYPE::BOOL, value };
    }
    char* value = new char[m_pos - start + 1];
    memcpy(value, m_text.data()+start, m_pos-start+1);
    value[m_pos - start + 1] = '\0';
    return { TOKEN_TYPE::IDENTIFIER, value };
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
