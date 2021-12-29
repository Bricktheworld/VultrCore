#include <gtest/gtest.h>
#include <utils/string_utils.h>

using namespace Vultr;
TEST(StringUtils, StartsWith)
{
	ASSERT_TRUE(starts_with("cat", 'c'));
	ASSERT_FALSE(starts_with("cat", 'C'));
	ASSERT_TRUE(starts_with("cat", 'C', CaseSensitivity::Insensitive));
	ASSERT_FALSE(starts_with("cat", '\0'));

	ASSERT_TRUE(starts_with("cat", "ca"));
	ASSERT_FALSE(starts_with("cat", "Ca"));
	ASSERT_TRUE(starts_with("cat", "Ca", CaseSensitivity::Insensitive));
	ASSERT_TRUE(starts_with("cat", "cat"));
	ASSERT_FALSE(starts_with("cat", "cat "));
}

TEST(StringUtils, EndsWith)
{
	ASSERT_TRUE(ends_with("cat", 't'));
	ASSERT_FALSE(ends_with("cat", 'T'));
	ASSERT_TRUE(ends_with("cat", 'T', CaseSensitivity::Insensitive));
	ASSERT_FALSE(ends_with("cat", '\0'));

	ASSERT_TRUE(ends_with("cat", "at"));
	ASSERT_FALSE(ends_with("cat", "At"));
	ASSERT_TRUE(ends_with("cat", "At", CaseSensitivity::Insensitive));
	ASSERT_TRUE(ends_with("cat", "cat"));
	ASSERT_FALSE(ends_with("cat", "cat "));
	ASSERT_FALSE(ends_with("cat", " cat"));
}

TEST(StringUtils, Contains)
{
	ASSERT_TRUE(contains("big chungus", 'c'));
	ASSERT_TRUE(contains("big chungus", ' '));
	ASSERT_FALSE(contains("big chungus", '\0'));
	ASSERT_FALSE(contains("big chungus", 'C'));
	ASSERT_TRUE(contains("big chungus", 'C', CaseSensitivity::Insensitive));

	ASSERT_TRUE(contains("big chungus", "chungus"));
	ASSERT_TRUE(contains("big chungus", " chungus"));
	ASSERT_TRUE(contains("big chungus", "big chungus"));
	ASSERT_FALSE(contains("big chungus", " big chungus"));
	ASSERT_FALSE(contains("big chungus", "Chungus"));
	ASSERT_TRUE(contains("big chungus", "Chungus", CaseSensitivity::Insensitive));
}

TEST(StringUtils, Find)
{
	ASSERT_FALSE(find("big chungus", 'a'));
	ASSERT_EQ(find("big chungus", 'b').value(), 0);
	ASSERT_EQ(find("big chungus", ' ').value(), 3);
	ASSERT_FALSE(find("big chungus", 'B'));
	ASSERT_FALSE(find("big chungus", '\0'));

	ASSERT_FALSE(find("big chungus", "Big"));
	ASSERT_EQ(find("big chungus", "big chungus").value(), 0);
	ASSERT_EQ(find("big chungus", "big chungus").value(), 0);
	ASSERT_FALSE(find("big chungus", "big chungus "));
	ASSERT_EQ(find("big chungus", " chungus").value(), 3);
}

TEST(StringUtils, FindLast)
{
	ASSERT_FALSE(find_last("big chungus", 'a'));
	ASSERT_EQ(find_last("big chungus", 'b').value(), 0);
}
