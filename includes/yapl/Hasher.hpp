//
// Created by lenin on 25.02.2026.
//

#pragma once

#include <openssl/evp.h>

#include <array>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <filesystem>

namespace yapl {

class Md5Hasher
{
public:
    Md5Hasher()
        : context(EVP_MD_CTX_new())
    {
        if (!context)
        {
            throw std::runtime_error("EVP_MD_CTX_new failed");
        }

        if (EVP_DigestInit_ex(context, EVP_md5(), nullptr) != 1)
        {
            EVP_MD_CTX_free(context);
            throw std::runtime_error("EVP_DigestInit_ex failed");
        }
    }

    ~Md5Hasher()
    {
        if (context)
        {
            EVP_MD_CTX_free(context);
        }
    }

    Md5Hasher(const Md5Hasher&) = delete;
    Md5Hasher& operator=(const Md5Hasher&) = delete;

    Md5Hasher(Md5Hasher&& other) noexcept
        : context(other.context)
    {
        other.context = nullptr;
    }

    Md5Hasher& operator=(Md5Hasher&& other) noexcept
    {
        if (this != &other)
        {
            if (context)
            {
                EVP_MD_CTX_free(context);
            }

            context = other.context;
            other.context = nullptr;
        }

        return *this;
    }

    void update(const void* data, std::size_t size)
    {
        if (size == 0)
        {
            return;
        }

        if (EVP_DigestUpdate(context, data, size) != 1)
        {
            throw std::runtime_error("EVP_DigestUpdate failed");
        }
    }

    void update(std::string_view text)
    {
        update(text.data(), text.size());
    }

    std::string finalHex()
    {
        std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
        unsigned int digestLength = 0;

        if (EVP_DigestFinal_ex(context, digest.data(), &digestLength) != 1)
        {
            throw std::runtime_error("EVP_DigestFinal_ex failed");
        }

        std::ostringstream output;
        output << std::hex << std::setfill('0');

        for (unsigned int i = 0; i < digestLength; ++i)
        {
            output << std::setw(2) << static_cast<unsigned int>(digest[i]);
        }

        reset();

        return output.str();
    }

    void reset()
    {
        if (EVP_DigestInit_ex(context, EVP_md5(), nullptr) != 1)
        {
            throw std::runtime_error("EVP_DigestInit_ex failed");
        }
    }

private:
    EVP_MD_CTX* context;
};

inline std::string md5HexFromFile(const std::filesystem::path& filePath, std::size_t bufferSize = 64 * 1024)
{
    std::ifstream fileStream(filePath, std::ios::binary);
    if (!fileStream)
    {
        throw std::runtime_error("Failed to open file: " + filePath.string());
    }

    if (bufferSize == 0)
    {
        throw std::runtime_error("bufferSize must be greater than 0");
    }

    Md5Hasher hasher;
    std::string buffer(bufferSize, '\0');

    while (fileStream)
    {
        fileStream.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize bytesRead = fileStream.gcount();

        if (bytesRead > 0)
        {
            hasher.update(buffer.data(), static_cast<std::size_t>(bytesRead));
        }
    }

    if (!fileStream.eof())
    {
        throw std::runtime_error("Error while reading file: " + filePath.string());
    }

    return hasher.finalHex();
}

inline std::string md5HexFromBytes(const void* data, std::size_t size)
{
    Md5Hasher hasher;
    hasher.update(data, size);
    return hasher.finalHex();
}

inline std::string md5HexFromString(std::string_view text)
{
    Md5Hasher hasher;
    hasher.update(text);
    return hasher.finalHex();
}

}
