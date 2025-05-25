#include <gtest/gtest.h>

#include <cstdint>

static uint64_t
Factorial(uint64_t number)
{
	return number <= 1 ? 1 : Factorial(number - 1) * number;
}

TEST(FactorialTest, baseTest)
{
	ASSERT_EQ(Factorial(0), 1);
	ASSERT_EQ(Factorial(1), 1);
	ASSERT_EQ(Factorial(2), 2);
	ASSERT_EQ(Factorial(3), 6);
	ASSERT_EQ(Factorial(10), 3628800);
}
