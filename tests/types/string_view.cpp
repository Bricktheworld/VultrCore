#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/string_view.h>

using namespace Vultr;
TEST(StringView, Create)
{
	{
		str example     = "Test";
		StringView view = example;
		ASSERT_STRCASEEQ(view.ref, example);
		ASSERT_TRUE(view == example);
		ASSERT_TRUE(example == view);
	}

	{
		String some_string("Test");
		StringView view = some_string;
		ASSERT_STRCASEEQ(view.ref, some_string.buffer);
		ASSERT_TRUE(view == some_string);
		ASSERT_TRUE(some_string == view);
	}
}
