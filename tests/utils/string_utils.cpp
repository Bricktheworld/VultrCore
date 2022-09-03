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

TEST(StringUtils, FindAll)
{
	auto indices = find_all("never gonna give you up, never gonna let you down", "never");
	ASSERT_EQ(indices.size(), 2);
	ASSERT_EQ(indices[0], 0);
}

TEST(StringUtils, ReplaceAll)
{
	auto res = replace_all("big big chungus, big chungus, big chungus", "big", "small");
	ASSERT_STRCASEEQ((str)res, "small small chungus, small chungus, small chungus");
}

TEST(StringUtils, Split)
{
	auto res = split("something, that, is, delimited", ", ");
	ASSERT_EQ(res.size(), 4);
	ASSERT_TRUE(res[0] == "something");
	ASSERT_TRUE(res[1] == "that");
	ASSERT_TRUE(res[2] == "is");
	ASSERT_TRUE(res[3] == "delimited");

	res = split("something, that, is, delimited, ", ", ");
	ASSERT_EQ(res.size(), 4);
	ASSERT_TRUE(res[0] == "something");
	ASSERT_TRUE(res[1] == "that");
	ASSERT_TRUE(res[2] == "is");
	ASSERT_TRUE(res[3] == "delimited");

	res = split("something, that, is, delimited, a", ", ");
	ASSERT_EQ(res.size(), 5);
	ASSERT_TRUE(res[0] == "something");
	ASSERT_TRUE(res[1] == "that");
	ASSERT_TRUE(res[2] == "is");
	ASSERT_TRUE(res[3] == "delimited");
	ASSERT_TRUE(res[4] == "a");
}

TEST(StringUtils, ParseF64)
{
	f64 res = parse_f64("0.512").value();
	ASSERT_EQ(res, 0.512);

	res = parse_f64("-0.512").value();
	ASSERT_EQ(res, -0.512);

	res = parse_f64("1e-10").value();
	ASSERT_EQ(res, 1e-10);

	res = parse_f64("-10.125e-10").value();
	ASSERT_EQ(res, -10.125e-10);
}

TEST(StringUtils, ParseU64)
{
	ASSERT_TRUE(parse_u64("0.512").has_error());
	ASSERT_TRUE(parse_u64("-1249").has_error());
	u64 res = parse_u64("1249").value();
	ASSERT_EQ(res, 1249);

	res = parse_u64("1e10").value();
	ASSERT_EQ(res, 1e10);
}

TEST(StringUtils, ParseS64)
{
	ASSERT_TRUE(parse_s64("0.512").has_error());
	ASSERT_TRUE(parse_s64("1e-10").has_error());
	s64 res = parse_s64("1249").value();
	ASSERT_EQ(res, 1249);

	res = parse_s64("-1249").value();
	ASSERT_EQ(res, -1249);

	res = parse_s64("-1e10").value();
	ASSERT_EQ(res, -1e10);

	res = parse_s64("1e10").value();
	ASSERT_EQ(res, 1e10);
}
