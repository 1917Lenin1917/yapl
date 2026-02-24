//
// Created by lenin on 24.02.2026.
//

#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace yapl {

void appendByte(std::vector<std::byte>& buffer, std::byte value);
void appendUint16(std::vector<std::byte>& buffer, std::uint16_t value);
void appendUint32(std::vector<std::byte>& buffer, std::uint32_t value);
void appendStringBytes(std::vector<std::byte>& buffer, const std::string& text);
void appendFloat(std::vector<std::byte>& buffer, float value);

}
