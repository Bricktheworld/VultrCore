#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/string.h>

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
		ASSERT_STRCASEEQ(view.ref, some_string.storage);
		ASSERT_TRUE(view == some_string);
		ASSERT_TRUE(some_string == view);
	}
}

TEST(StringView, Substr)
{
	{
		str example     = "Test";
		StringView view = example;
		auto substr     = view.substr(0);
		ASSERT_STRCASEEQ(substr.c_str(), example);
		substr = view.substr(1);
		ASSERT_STRCASEEQ(substr.c_str(), "est");
		substr = view.substr(0, 4);
		ASSERT_STRCASEEQ(substr.c_str(), example);
	}
}
