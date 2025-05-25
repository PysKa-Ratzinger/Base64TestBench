#include "Base64Codec.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <span>
#include <string>
#include <vector>

namespace
{} // anonymous namespace

bool
Base64Codec::isValidChar(char code)
{
	return (code >= 'A' && code <= 'Z') || (code >= 'a' && code <= 'z') ||
	       (code >= '0' && code <= '9') || (code == '+' || code == '/');
}

char
Base64Codec::toB64Char(std::byte a_c)
{
	int chr = int(a_c);
	if (chr < 26) {
		return char(int('A') + chr);
	}
	if (chr < 52) {
		return char(int('a') + chr - 26);
	}
	if (chr < 62) {
		return char(int('0') + chr - 52);
	}
	if (chr == 62) {
		return '+';
	}
	if (chr == 63) {
		return '/';
	}
	return '?';
}

std::byte
Base64Codec::fromB64Char(char a_c)
{
	if (a_c >= 'A' && a_c <= 'Z') {
		return std::byte(a_c - 'A');
	}
	if (a_c >= 'a' && a_c <= 'z') {
		return std::byte(a_c - 'a' + 26);
	}
	if (a_c >= '0' && a_c <= '9') {
		return std::byte(a_c - '0' + 52);
	}
	if (a_c == '+') {
		return std::byte(62);
	}
	if (a_c == '/') {
		return std::byte(63);
	}
	return std::byte(0);
}

size_t
Base64Codec::getEncodedLength(size_t dec_len)
{
	size_t n_blocks = (dec_len + (3 - 1)) / 3;
	return n_blocks * 4;
}

size_t
Base64Codec::getDecodedLength(size_t enc_len)
{
	static constexpr std::array<size_t, 4> remaining_bytes{
		0,
		0,
		1,
		2,
	};
	size_t n_blocks = enc_len / 4;
	return (n_blocks * 3) + remaining_bytes.at(enc_len % 4);
}

std::vector<std::byte>
Base64Codec::decode(std::span<const char> data)
{
	size_t data_sz = 0;
	for (const auto& data_c : data) {
		if (isValidChar(data_c)) {
			data_sz++;
		} else {
			break;
		}
	}

	std::vector<std::byte> result;

	std::vector<std::byte> char_array_4{};
	std::vector<std::byte> char_array_3{};
	for (size_t i = 0; i < data_sz; i += 4) {
		char_array_4.clear();
		char_array_3.clear();

		for (size_t j = 0; j < 4; j++) {
			if (i + j >= data_sz) {
				char_array_4.emplace_back(std::byte(0));
			} else {
				char_array_4.emplace_back(
				        Base64Codec::fromB64Char(data[i + j])
				);
			}
		}

		char_array_3 = Base64Codec::decodeBlock(char_array_4);

		result.push_back(char_array_3[0]);
		result.push_back(char_array_3[1]);
		result.push_back(char_array_3[2]);
	}

	size_t n_pads = 0;
	switch (data_sz % 4) {
	case 0:
		n_pads = 0;
		break;
	case 1:
		n_pads = 3;
		break;
	case 2:
		n_pads = 2;
		break;
	case 3:
		n_pads = 1;
		break;
	}

	for (size_t i = 0; i < n_pads; i++) {
		result.pop_back();
	}

	return result;
}

std::vector<std::byte>
Base64Codec::decodeBlock(std::vector<std::byte> block)
{
	if (block.size() < 4) {
		throw std::runtime_error("Expected block size to be at least 4");
	}

	std::vector<std::byte> result;

	result.emplace_back(
	        ((block.at(0) << 2U) | ((block.at(1) & std::byte(0x30)) >> 4U))
	);

	result.emplace_back(
	        (((block.at(1) & std::byte(0xf)) << 4U) |
	         ((block.at(2) & std::byte(0x3c)) >> 2U))
	);

	result.emplace_back(
	        (((block.at(2) & std::byte(0x3)) << 6U) | block.at(3))
	);

	return result;
}

std::string
Base64Codec::encode(std::span<const std::byte> data)
{
	std::string result;
	result.resize(getEncodedLength(data.size()));
	encode(data, std::span(result));
	return result;
}

std::string
Base64Codec::encode(std::span<const char> data)
{
	return encode(std::as_bytes(data));
}

size_t
Base64Codec::encode(std::span<const std::byte> data, std::span<char> output)
{
	size_t result_sz = Base64Codec::getEncodedLength(data.size());
	if (output.size() < result_sz) {
		return 0;
	}

	std::array<std::byte, 3> char_array_3{};
	std::array<std::byte, 4> char_array_4{};

	size_t output_i = 0;

	for (size_t i = 0; i < data.size(); i += 3) {
		std::ranges::fill(char_array_3, std::byte(0));
		size_t max_iterations_block = std::min(3LU, data.size() - i);
		for (size_t j = 0; j < max_iterations_block; j++) {
			char_array_3.at(j) = data.at(i + j);
		}

		// 0000 0000 1111 1111 2222 2222
		// AAAA AABB BBBB CCCC CCDD DDDD

		char_array_4.at(0) = (char_array_3.at(0) & std::byte(0xFC)) >>
		                     2U;
		char_array_4.at(1) =
		        ((char_array_3.at(0) & std::byte(0x03)) << 4U) |
		        ((char_array_3.at(1) & std::byte(0xF0)) >> 4U);
		char_array_4.at(2) =
		        ((char_array_3.at(1) & std::byte(0x0FU)) << 2U) |
		        ((char_array_3.at(2) & std::byte(0xC0U)) >> 6U);
		char_array_4.at(3) = char_array_3.at(2) & std::byte(0x3FU);

		for (size_t j = 0; j < 4; j++) {
			output.at(output_i++) =
			        Base64Codec::toB64Char(char_array_4.at(j));
		}
	}

	size_t n_pads = (3 - (data.size() % 3)) % 3;

	for (size_t i = output_i - n_pads; i < output_i; i++) {
		output.at(i) = '=';
	}

	return output_i;
}
