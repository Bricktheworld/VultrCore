#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/hashtable.h>
#include <types/string.h>
#include "utils/string_utils.h"

using namespace Vultr;

TEST(HashTable, Iterator)
{
	//	const char *string_keys[] = {"urmom", "chungus", "poop", "awoooga"};
	const char *string_vals[] = {"joe", "big", "shit", "eyes pop out"};
	HashTable<String> table{};

	for (int i = 0; i < 4; i++)
	{
		table.set(String(string_vals[i]));
	}

	ASSERT_TRUE(table.contains("joe"));
	ASSERT_TRUE(table.contains("big"));
	ASSERT_TRUE(table.contains("shit"));
	ASSERT_TRUE(table.contains("eyes pop out"));
	ASSERT_FALSE(table.contains("ajoe"));
	ASSERT_FALSE(table.contains(""));

	ASSERT_TRUE(table.remove("joe"));
	ASSERT_EQ(table.size(), 3);
	ASSERT_FALSE(table.contains("joe"));
	ASSERT_FALSE(table.remove("joe"));

	ASSERT_EQ(table.remove_all_matching([](const String &data) -> bool { return contains(data, 'i'); }), 2);
	ASSERT_EQ(table.size(), 1);
	ASSERT_FALSE(table.contains("big"));
	ASSERT_FALSE(table.contains("shit"));

	table.remove(table.begin());
	ASSERT_EQ(table.size(), 0);
	ASSERT_FALSE(table.contains("eyes pop out"));
	ASSERT_EQ(table.begin(), table.end());

	// Re-enter all of the values so that we can test destruction.
	for (int i = 0; i < 4; i++)
	{
		table.set(String(string_vals[i]));
	}
}
