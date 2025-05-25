#include "Base64Codec.hpp"

#include <gtest/gtest.h>
#include <stdlib.h>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <random>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace
{

auto
toVU8(const std::string& str) -> std::vector<std::byte>
{
	std::vector<std::byte> result;
	std::ranges::copy(
	        std::as_bytes(std::span(str)), std::back_inserter(result)
	);
	return result;
}

}

TEST(Base64, getEncodedLength)
{
	EXPECT_EQ(Base64Codec::getEncodedLength(0), 0);
	EXPECT_EQ(Base64Codec::getEncodedLength(1), 4);
	EXPECT_EQ(Base64Codec::getEncodedLength(2), 4);
	EXPECT_EQ(Base64Codec::getEncodedLength(3), 4);
	EXPECT_EQ(Base64Codec::getEncodedLength(4), 8);
	EXPECT_EQ(Base64Codec::getEncodedLength(5), 8);
	EXPECT_EQ(Base64Codec::getEncodedLength(6), 8);
}

TEST(Base64, getDecodedLength)
{
	EXPECT_EQ(Base64Codec::getDecodedLength(0), 0);
	EXPECT_EQ(Base64Codec::getDecodedLength(1), 0);
	EXPECT_EQ(Base64Codec::getDecodedLength(2), 1);
	EXPECT_EQ(Base64Codec::getDecodedLength(3), 2);
	EXPECT_EQ(Base64Codec::getDecodedLength(4), 3);
	EXPECT_EQ(Base64Codec::getDecodedLength(5), 3);
	EXPECT_EQ(Base64Codec::getDecodedLength(6), 4);
	EXPECT_EQ(Base64Codec::getDecodedLength(7), 5);
	EXPECT_EQ(Base64Codec::getDecodedLength(8), 6);
}

TEST(Base64, decode)
{
	auto v_1 = toVU8("Hello there.");
	auto v_2 = toVU8("Hello there..");
	auto v_3 = toVU8("Hello there...");
	auto v_4 = toVU8("Hello there....");

	EXPECT_EQ(v_1, Base64Codec::decode("SGVsbG8gdGhlcmUu"));
	EXPECT_EQ(v_2, Base64Codec::decode("SGVsbG8gdGhlcmUuLg=="));
	EXPECT_EQ(v_3, Base64Codec::decode("SGVsbG8gdGhlcmUuLi4="));
	EXPECT_EQ(v_4, Base64Codec::decode("SGVsbG8gdGhlcmUuLi4u"));
}

TEST(Base64, decodeMissingPadding)
{
	auto v_1 = toVU8("Hello there.");
	auto v_2 = toVU8("Hello there..");
	auto v_3 = toVU8("Hello there...");
	auto v_4 = toVU8("Hello there....");

	EXPECT_EQ(v_1, Base64Codec::decode("SGVsbG8gdGhlcmUu"));
	EXPECT_EQ(v_2, Base64Codec::decode("SGVsbG8gdGhlcmUuLg"));
	EXPECT_EQ(v_3, Base64Codec::decode("SGVsbG8gdGhlcmUuLi4"));
	EXPECT_EQ(v_4, Base64Codec::decode("SGVsbG8gdGhlcmUuLi4u"));
}

TEST(Base64, encode)
{
	auto v_1 = toVU8("Hello there.");
	auto v_2 = toVU8("Hello there..");
	auto v_3 = toVU8("Hello there...");
	auto v_4 = toVU8("Hello there....");

	EXPECT_EQ(Base64Codec::encode(v_1), "SGVsbG8gdGhlcmUu");
	EXPECT_EQ(Base64Codec::encode(v_2), "SGVsbG8gdGhlcmUuLg==");
	EXPECT_EQ(Base64Codec::encode(v_3), "SGVsbG8gdGhlcmUuLi4=");
	EXPECT_EQ(Base64Codec::encode(v_4), "SGVsbG8gdGhlcmUuLi4u");
}

TEST(Base64, longRandomInputDoesNotCrash)
{
	std::random_device dev;
	std::mt19937       rng{ dev() };

	for (size_t i = 1000; i < 1200; i++) {
		std::vector<std::byte> input;
		input.resize(i);

		for (std::byte& chr : input) {
			chr = std::byte(rng());
		}

		auto enc_buf = Base64Codec::encode(input);
		auto dec_buf = Base64Codec::decode(enc_buf);

		ASSERT_EQ(dec_buf, input);
	}
}
