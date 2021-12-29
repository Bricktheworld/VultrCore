#include <gtest/gtest.h>
#include <types/error.h>

using namespace Vultr;
TEST(Error, Create)
{
	{
		auto err = Error();
		ASSERT_TRUE(err);
	}
	{
		auto err = Error("Some message");
		ASSERT_STRCASEEQ(err.message, "Some message");
		ASSERT_EQ(err.code, -1);
	}
	{
		auto err = Error("Some message", 20);
		ASSERT_STRCASEEQ(err.message, "Some message");
		ASSERT_EQ(err.code, 20);
	}
}
