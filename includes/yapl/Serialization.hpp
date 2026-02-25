//
// Created by lenin on 24.02.2026.
//

#pragma once
#include <string>
#include <vector>
#include <cstdint>

#include "values/TypeObject.hpp"

namespace yapl {

void appendByte(std::vector<std::byte>& buffer, std::byte value);
void appendUint16(std::vector<std::byte>& buffer, std::uint16_t value);
void appendUint32(std::vector<std::byte>& buffer, std::uint32_t value);
void appendStringBytes(std::vector<std::byte>& buffer, const std::string& text);
void appendFloat(std::vector<std::byte>& buffer, float value);

void ensureAvailable(const std::vector<std::byte>& buffer, std::size_t offset, std::size_t needed);
std::byte readByte(const std::vector<std::byte>& buffer, std::size_t& offset);
std::uint16_t readUint16(const std::vector<std::byte>& buffer, std::size_t& offset);
std::uint32_t readUint32(const std::vector<std::byte>& buffer, std::size_t& offset);
std::string readStringBytes(const std::vector<std::byte>& buffer, std::size_t& offset, std::size_t length);
float readFloat(const std::vector<std::byte>& buffer, std::size_t& offset);
VPtr readValue(const std::vector<std::byte>& buffer, std::size_t& offset);

}
