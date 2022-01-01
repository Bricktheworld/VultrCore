#include <gtest/gtest.h>
#include <types/string.h>
#include "types/string_view.h"

using namespace Vultr;
TEST(String, Create)
{
	{
		String string;
		ASSERT_EQ(string.size(), 1);
	}
	{
		String string("Something");
		ASSERT_EQ(string.size(), strlen("Something") + 1);
	}
	{
		String string("Something", 2);
		ASSERT_EQ(string.size(), strlen("So") + 1);
	}
	{
		String some_string("something");
		{
			auto other  = String("new_string");
			some_string = other;
			ASSERT_NE(other.storage, some_string.storage);
			ASSERT_STRCASEEQ(some_string, "new_string");
		}
		ASSERT_STRCASEEQ(some_string, "new_string");
	}
}

TEST(String, Concat)
{
	{
		String string("Something");
		string += " something else";
		ASSERT_EQ(string.size(), strlen("Something something else") + 1);
		ASSERT_STRCASEEQ(string.storage, "Something something else");

		String new_str = StringView(string) + " chungus";
		ASSERT_EQ(new_str.size(), strlen("Something something else chungus") + 1);
		ASSERT_STRCASEEQ(new_str.storage, "Something something else chungus");

		String other_str;
		other_str += StringView("hello");
		ASSERT_EQ(other_str.size(), strlen("hello") + 1);
		ASSERT_STRCASEEQ(other_str.storage, "hello");
	}
}

TEST(String, Equality)
{
	{
		String string("Something");
		str other = "Something";
		ASSERT_STRCASEEQ(string.storage, other);
		ASSERT_TRUE(string == other);
	}
	{
		String string("Something");
		String other("Something");
		ASSERT_STRCASEEQ(string.storage, other);
		ASSERT_TRUE(string == other);
	}
}