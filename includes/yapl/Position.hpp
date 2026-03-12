//
// Created by lenin on 07.03.2026.
//

#pragma once
#include <cstddef>

namespace yapl {

struct Position
{
  std::size_t line;
  std::size_t character;
};

struct Range
{
  Position start;
  Position end;
};

struct Location
{
  std::size_t file_id;
  Range range;
};


}
