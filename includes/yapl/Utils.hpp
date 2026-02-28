//
// Created by lenin on 26.02.2026.
//

#pragma once

#include <cassert>
#include <filesystem>
#include <vector>

#include "Hasher.hpp"
#include "Serialization.hpp"
constexpr std::uint8_t MODULE_VERSION = 1;

namespace yapl {
inline bool is_valid_cache(const std::filesystem::path& module_path)
{
  auto cache_path = module_path.parent_path() / ".cache" / module_path.filename().replace_extension("yaplcache");
  auto new_md5 = md5HexFromFile(module_path);
  std::ifstream t(cache_path, std::ios::binary);
  if (!t.is_open())
    return false;

  std::uint8_t version = 0;
  t.read(reinterpret_cast<std::istream::char_type *>(&version), 1);

  assert(("Module version is wrong!", version == MODULE_VERSION));

  std::string old_md5;
  old_md5.resize(32);
  t.read(old_md5.data(), 32);

  t.close();

  return old_md5 == new_md5;
}

inline std::vector<std::byte> serialize_module(const std::vector<std::byte>& co_bytes, const std::filesystem::path& module_path)
{
  std::vector<std::byte> result;
  appendByte(result, static_cast<std::byte>(MODULE_VERSION));
  const auto hash = md5HexFromFile(module_path);
  appendStringBytes(result, hash);
  result.insert(result.end(), co_bytes.begin(), co_bytes.end());

  return result;
}

inline std::vector<std::string> get_lines_from_text(const std::string& text)
{
    std::vector<std::string> res;
    std::stringstream ss { text };
    while (!ss.eof())
    {
        std::string line;
        std::getline(ss, line);
        res.push_back(line);
    }
    return res;
}

}
