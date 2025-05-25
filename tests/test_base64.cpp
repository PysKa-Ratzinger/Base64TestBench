#include <gtest/gtest.h>

#include "Base64Codec.hpp"

TEST(Base64, encodedStringIsNotEqualsToItself) {
	ASSERT_NE(Base64Codec::encode("asdf"), "asdf");
}
