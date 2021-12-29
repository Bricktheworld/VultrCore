#include <gtest/gtest.h>
#include <types/error_or.h>

using namespace Vultr;
TEST(ErrorOr, Create)
{
	{
		ErrorOr<u32> result = Error("An error", -1);
		bool worked         = false;
		if check (result, auto number, auto err)
		{
			worked = false;
		}
		else
		{
			worked = true;
		}
		ASSERT_TRUE(worked);
	}
	{
		ErrorOr<u32> result = 20;
		bool worked         = false;
		if check (result, auto number, auto err)
		{
			worked = true;
			ASSERT_EQ(number, 20);
		}
		else
		{
			worked = false;
		}
		ASSERT_TRUE(worked);
	}
}
