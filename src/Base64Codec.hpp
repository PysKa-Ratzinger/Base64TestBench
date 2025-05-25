#pragma once

#include <cstddef>
#include <span>
#include <vector>

class Base64Codec
{
public:
	static bool isValidChar(char code);

	static char toB64Char(std::byte a_c);

	static std::byte fromB64Char(char a_c);

	static size_t getEncodedLength(size_t dec_len);

	static size_t getDecodedLength(size_t enc_len);

	static std::vector<std::byte> decode(std::span<const char> data);

	static std::string encode(std::span<const std::byte> data);

private:
	static std::vector<std::byte> decodeBlock(std::vector<std::byte> block);

	static std::string encode(std::span<const char> data);
	static size_t
	encode(std::span<const std::byte> data, std::span<char> output);
};
