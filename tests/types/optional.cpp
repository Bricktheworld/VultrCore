#include <gtest/gtest.h>
#include <types/optional.h>

using namespace Vultr;
TEST(Option, Creation)
{
	Option<u32> number;
	ASSERT_FALSE(number);

	number = 0;
	ASSERT_TRUE(number);
	ASSERT_EQ(number.value(), 0);

	bool worked;
	if let (auto &value, number)
	{
		worked = true;
	}
	else
	{
		worked = false;
	}
	ASSERT_TRUE(worked);
}
