//
// Created by lenin on 24.02.2026.
//

#include "yapl/Serialization.hpp"

#include <stdexcept>
#include <yapl/Variable.hpp>
#include <yapl/values/BooleanValue.hpp>
#include <yapl/values/IntegerValue.hpp>
#include <yapl/values/TypeObject.hpp>

#include "yapl/values/CodeObjectValue.hpp"

namespace yapl {

void appendByte(std::vector<std::byte> &buffer, std::byte value)
{
  buffer.push_back(value);
}

void appendUint16(std::vector<std::byte>& buffer, std::uint16_t value)
{
  buffer.push_back(static_cast<std::byte>(value & 0xFFu));
  buffer.push_back(static_cast<std::byte>((value >> 8) & 0xFFu));
}

void appendUint32(std::vector<std::byte> &buffer, std::uint32_t value)
{
  buffer.push_back(static_cast<std::byte>(value & 0xFFu));
  buffer.push_back(static_cast<std::byte>((value >> 8) & 0xFFu));
  buffer.push_back(static_cast<std::byte>((value >> 16) & 0xFFu));
  buffer.push_back(static_cast<std::byte>((value >> 24) & 0xFFu));
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


void ensureAvailable(const std::vector<std::byte>& buffer, std::size_t offset, std::size_t needed)
{
    if (offset + needed > buffer.size())
    {
        throw std::out_of_range("Not enough bytes in buffer");
    }
}

std::byte readByte(const std::vector<std::byte>& buffer, std::size_t& offset)
{
    ensureAvailable(buffer, offset, 1);
    return buffer[offset++];
}

std::uint16_t readUint16(const std::vector<std::byte>& buffer, std::size_t& offset)
{
    ensureAvailable(buffer, offset, 2);

    const auto byte0 = static_cast<std::uint16_t>(std::to_integer<unsigned char>(buffer[offset]));
    const auto byte1 = static_cast<std::uint16_t>(std::to_integer<unsigned char>(buffer[offset + 1]));
    offset += 2;

    return static_cast<std::uint16_t>(byte0 | (byte1 << 8));
}

std::uint32_t readUint32(const std::vector<std::byte>& buffer, std::size_t& offset)
{
    ensureAvailable(buffer, offset, 4);

    const auto byte0 = static_cast<std::uint32_t>(std::to_integer<unsigned char>(buffer[offset]));
    const auto byte1 = static_cast<std::uint32_t>(std::to_integer<unsigned char>(buffer[offset + 1]));
    const auto byte2 = static_cast<std::uint32_t>(std::to_integer<unsigned char>(buffer[offset + 2]));
    const auto byte3 = static_cast<std::uint32_t>(std::to_integer<unsigned char>(buffer[offset + 3]));
    offset += 4;

    return byte0 | (byte1 << 8) | (byte2 << 16) | (byte3 << 24);
}

std::string readStringBytes(const std::vector<std::byte>& buffer, std::size_t& offset, std::size_t length)
{
    ensureAvailable(buffer, offset, length);

    std::string text;
    text.reserve(length);

    for (std::size_t i = 0; i < length; ++i)
    {
        text.push_back(static_cast<char>(std::to_integer<unsigned char>(buffer[offset + i])));
    }

    offset += length;
    return text;
}

float readFloat(const std::vector<std::byte>& buffer, std::size_t& offset)
{
    const auto bitPattern = readUint32(buffer, offset);
    return std::bit_cast<float>(bitPattern);
}

VPtr readValue(const std::vector<std::byte>& buffer, std::size_t& offset)
{
    switch (auto type = static_cast<VALUE_TYPE>(readByte(buffer, offset)))
    {
        case VALUE_TYPE::BOOL: return BooleanValue::Deserialize(buffer, offset);
        case VALUE_TYPE::INTEGER: return IntegerValue::Deserialize(buffer, offset);
        case VALUE_TYPE::FLOAT: return FloatValue::Deserialize(buffer, offset);
        case VALUE_TYPE::STRING: return StringValue::Deserialize(buffer, offset);
        case VALUE_TYPE::CODE_OBJECT: return CodeObjectValue::Deserialize(buffer, offset);
        default: assert(("Invalid value type", true));
    }
    return nullptr;
}

}
