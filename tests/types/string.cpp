#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/string.h>

using namespace Vultr;
TEST(String, Create)
{
	{
		String string;
		ASSERT_EQ(string.size, 0);
		ASSERT_EQ(string.buffer, nullptr);
		ASSERT_EQ(string.allocator, g_game_memory->general_allocator);
	}
	{
		String string = "Something";
		ASSERT_EQ(string.size, strlen("Something") + 1);
		ASSERT_NE(string.buffer, nullptr);
		ASSERT_EQ(string.allocator, g_game_memory->general_allocator);
	}
	{
		String string("Something", 2);
		ASSERT_EQ(string.size, strlen("So") + 1);
		ASSERT_NE(string.buffer, nullptr);
		ASSERT_EQ(string.allocator, g_game_memory->general_allocator);
	}
}

TEST(String, Concat)
{
	{
		String string("Something");
		string += " something else";
		ASSERT_EQ(string.size, strlen("Something something else") + 1);
		ASSERT_STRCASEEQ(string.buffer, "Something something else");

		str str1       = " chungus";
		size_t len     = strlen(str1);
		String new_str = string + str1;
		ASSERT_EQ(new_str.size, strlen("Something something else chungus") + 1);
		ASSERT_STRCASEEQ(new_str.buffer, "Something something else chungus");
	}
}

TEST(String, Cast)
{
	String string = "Something";
	str data      = string;
	ASSERT_STRCASEEQ(data, string.buffer);
}

TEST(String, Equality)
{
	{
		String string = "Something";
		str other     = "Something";
		ASSERT_STRCASEEQ(string.buffer, other);
		ASSERT_TRUE(string == other);
	}
	{
		String string = "Something";
		String other  = "Something";
		ASSERT_STRCASEEQ(string.buffer, other);
		ASSERT_TRUE(string == other);
	}
}