//
// Created by lenin on 24.02.2026.
//

#include "yapl/Serialization.hpp"

namespace yapl {

void appendByte(std::vector<std::byte> &buffer, std::byte value)
{
  buffer.push_back(value);
}

void appendUint16(std::vector<std::byte>& buffer, std::uint16_t value)
{
    if constexpr (std::endian::native == std::endian::little)
    {
        buffer.push_back(static_cast<std::byte>(value & 0xFFu));
        buffer.push_back(static_cast<std::byte>((value >> 8) & 0xFFu));
    }
    else
    {
        buffer.push_back(static_cast<std::byte>((value >> 8) & 0xFFu));
        buffer.push_back(static_cast<std::byte>(value & 0xFFu));
    }
}

void appendUint32(std::vector<std::byte> &buffer, std::uint32_t value)
{
  if constexpr (std::endian::native == std::endian::little)
  {
      buffer.push_back(static_cast<std::byte>(value & 0xFFu));
      buffer.push_back(static_cast<std::byte>((value >> 8) & 0xFFu));
      buffer.push_back(static_cast<std::byte>((value >> 16) & 0xFFu));
      buffer.push_back(static_cast<std::byte>((value >> 24) & 0xFFu));
  }
  else
  {
      buffer.push_back(static_cast<std::byte>((value >> 24) & 0xFFu));
      buffer.push_back(static_cast<std::byte>((value >> 16) & 0xFFu));
      buffer.push_back(static_cast<std::byte>((value >> 8) & 0xFFu));
      buffer.push_back(static_cast<std::byte>(value & 0xFFu));
  }
}

void appendStringBytes(std::vector<std::byte> &buffer, const std::string &text)
{
  buffer.reserve(buffer.size() + text.size());
  for (unsigned char character : text)
  {
      buffer.push_back(static_cast<std::byte>(character));
  }
}

void appendFloat(std::vector<std::byte>& buffer, float value)
{
    const auto bitPattern = std::bit_cast<std::uint32_t>(value);
    appendUint32(buffer, bitPattern);
}


}