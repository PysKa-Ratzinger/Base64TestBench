#include <gtest/gtest.h>
#include <iostream>

auto
main(int argc, char* argv[]) -> int
{
	::testing::InitGoogleTest(&argc, argv);
	int err = RUN_ALL_TESTS();
	return err;
}
