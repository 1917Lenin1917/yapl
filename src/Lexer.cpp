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
        switch (char c = m_text[m_pos])
        {
            case '\0': { break; }
            case '\t':
            case ' ': { continue; }
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9': { tokens.push_back(make_number()); break; }
            case '+': { tokens.emplace_back(TOKEN_TYPE::PLUS); break; }
            case '-': { tokens.emplace_back(TOKEN_TYPE::MINUS); break; }
            case '*': { tokens.emplace_back(TOKEN_TYPE::MUL); break; }
            case '/': { tokens.emplace_back(TOKEN_TYPE::DIV); break; }
            case '(': { tokens.emplace_back(TOKEN_TYPE::LPAREN); break; }
            case ')': { tokens.emplace_back(TOKEN_TYPE::RPAREN); break; }
            default: assert(false);
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


} // yapl