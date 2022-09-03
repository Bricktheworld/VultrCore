#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/hashmap.h>
#include <types/string.h>
#include "utils/string_utils.h"

using namespace Vultr;

TEST(Hashmap, Iterator)
{
	const char *string_keys[] = {"urmom", "chungus", "poop", "awoooga"};
	const char *string_vals[] = {"joe", "big", "shit", "eyes pop out"};
	Hashmap<String, String> map{};

	for (int i = 0; i < 4; i++)
	{
		map.set(string_keys[i], String(string_vals[i]));
	}

	ASSERT_EQ(map.size(), 4);
	for (auto &[key, value] : map)
	{
	}

	ASSERT_TRUE(map.remove("urmom"));
	ASSERT_FALSE(map.contains("urmom"));
	auto &value = map.get("chungus");
	value += " chungus";
	ASSERT_STRCASEEQ(map.get("chungus").c_str(), "big chungus");
	map.remove("chungus");
	ASSERT_EQ(value.size(), 0);

	ASSERT_EQ(map.remove_all_matching([](const String &key, const String &value) { return contains(key, "p"); }), 1);
	ASSERT_FALSE(map.contains("poop"));
}