#include <gtest/gtest.h>
#include <types/optional.h>

using namespace Vultr;
TEST(Option, Creation)
{
	Option<u32> number;
	ASSERT_FALSE(number);

	number = 20;
	ASSERT_TRUE(number);
	ASSERT_EQ(number.value(), 20);

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
