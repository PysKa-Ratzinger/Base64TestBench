#include "Base64Codec.hpp"

#include <gtest/gtest.h>

TEST(Base64, encodedStringIsNotEqualsToItself)
{
	ASSERT_NE(Base64Codec::encode("asdf"), "asdf");
}
