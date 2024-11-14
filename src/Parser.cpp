//
// Created by lenin on 14.11.2024.
//

#include "Parser.hpp"

namespace yapl {
std::unique_ptr<BaseASTNode> Parser::generate_ast()
{

}

Token Parser::advance()
{
  m_pos++;
  return m_tokens[m_pos];
}


}