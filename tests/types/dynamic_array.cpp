#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/dynamic_array.h>
#include <vultr.h>

using namespace vtl;
using namespace Vultr;
class DynamicArrayTests : public testing::Test
{
  protected:
	// Per-test-suite set-up.
	// Called before the first test in this test suite.
	// Can be omitted if not needed.
	static void SetUpTestSuite() {}

	// Per-test-suite tear-down.
	// Called after the last test in this test suite.
	// Can be omitted if not needed.
	static void TearDownTestSuite() {}
};

TEST_F(DynamicArrayTests, EmptyInitialize)
{
	DynamicArray<const char *, 15> string_list;
	ASSERT_EQ(string_list._size, 15);
	ASSERT_EQ(string_list.len, 0);
	ASSERT_NE(string_list._array, nullptr);
}

TEST_F(DynamicArrayTests, ArrayInitialize)
{
	const char *string_array[] = {"hello", "world"};
	DynamicArray<const char *, 10> string_list(string_array, 2);
	ASSERT_EQ(string_list._size, 10);
	ASSERT_EQ(string_list.len, 2);
	ASSERT_NE(string_list._array, nullptr);
}

TEST_F(DynamicArrayTests, Operator)
{
	const char *string_array[] = {"hello"};
	DynamicArray<const char *> string_list(string_array, 1);
	ASSERT_EQ("hello", string_list[0]);
	string_list[0] = "world";
	ASSERT_EQ("world", string_list[0]);
	ASSERT_DEATH(string_list[1] = "ur mother", "Index out of bounds");
}

TEST_F(DynamicArrayTests, PushBack)
{
	const char *string_array[] = {"hello"};
	DynamicArray<const char *, 10> string_list(string_array, 1);

	string_list.push_back("world");
	ASSERT_EQ(string_list[1], "world");
	ASSERT_EQ(string_list._size, 10);
	ASSERT_EQ(string_list.len, 2);
}

TEST_F(DynamicArrayTests, Insert)
{
	const char *string_array[] = {"big", "big", "chungus", "big", "chungus"};
	DynamicArray<const char *, 10> string_list(string_array, 5);

	// Insert middle
	string_list.insert(1, "joe");
	ASSERT_EQ(string_list[0], "big");
	ASSERT_EQ(string_list[1], "joe");
	ASSERT_EQ(string_list[2], "big");
	ASSERT_EQ(string_list[3], "chungus");
	ASSERT_EQ(string_list[4], "big");
	ASSERT_EQ(string_list[5], "chungus");
	ASSERT_EQ(string_list._size, 10);
	ASSERT_EQ(string_list.len, 6);

	// Insert beginning
	string_list.insert(0, "mama");
	ASSERT_EQ(string_list[0], "mama");
	ASSERT_EQ(string_list[1], "big");
	ASSERT_EQ(string_list[2], "joe");
	ASSERT_EQ(string_list[3], "big");
	ASSERT_EQ(string_list[4], "chungus");
	ASSERT_EQ(string_list[5], "big");
	ASSERT_EQ(string_list[6], "chungus");
	ASSERT_EQ(string_list._size, 10);
	ASSERT_EQ(string_list.len, 7);

	// Insert end / push_back
	string_list.insert(7, "urmother");
	ASSERT_EQ(string_list[0], "mama");
	ASSERT_EQ(string_list[1], "big");
	ASSERT_EQ(string_list[2], "joe");
	ASSERT_EQ(string_list[3], "big");
	ASSERT_EQ(string_list[4], "chungus");
	ASSERT_EQ(string_list[5], "big");
	ASSERT_EQ(string_list[6], "chungus");
	ASSERT_EQ(string_list[7], "urmother");
	ASSERT_EQ(string_list._size, 10);
	ASSERT_EQ(string_list.len, 8);

	string_list.insert(0, "mama");
	string_list.insert(0, "mama");
	string_list.insert(0, "mama");
	ASSERT_EQ(string_list[0], "mama");
	ASSERT_EQ(string_list[1], "mama");
	ASSERT_EQ(string_list[2], "mama");
	ASSERT_EQ(string_list[3], "mama");
	ASSERT_EQ(string_list[4], "big");
	ASSERT_EQ(string_list[5], "joe");
	ASSERT_EQ(string_list[6], "big");
	ASSERT_EQ(string_list[7], "chungus");
	ASSERT_EQ(string_list[8], "big");
	ASSERT_EQ(string_list[9], "chungus");
	ASSERT_EQ(string_list[10], "urmother");
	ASSERT_EQ(string_list._size, (int)(11 * 3 / 2));
	ASSERT_EQ(string_list.len, 11);
}

TEST_F(DynamicArrayTests, Delete)
{
	const char *string_array[] = {"big", "big", "chungus", "big", "chungus"};

	const u32 decay_percentage = 50;
	DynamicArray<const char *, 0, 3, 2, decay_percentage> string_list(string_array, 5);

	ASSERT_EQ(string_list._size, (int)5 * 3 / 2);
	ASSERT_EQ(string_list.len, 5);

	string_list.remove(1);
	ASSERT_EQ(string_list[0], "big");
	ASSERT_EQ(string_list[1], "chungus");
	ASSERT_EQ(string_list[2], "big");
	ASSERT_EQ(string_list[3], "chungus");
	ASSERT_EQ(string_list._size, (int)5 * 3 / 2);
	ASSERT_EQ(string_list.len, 4);

	// Less than decay factor 50%
	string_list.remove(3);
	ASSERT_EQ(string_list[0], "big");
	ASSERT_EQ(string_list[1], "chungus");
	ASSERT_EQ(string_list[2], "big");

	ASSERT_EQ(string_list._size, (int)3 * 3 / 2);
	ASSERT_EQ(string_list.len, 3);

	string_list.remove(0);
	ASSERT_EQ(string_list[0], "chungus");
	ASSERT_EQ(string_list[1], "big");
	ASSERT_EQ(string_list._size, (int)3 * 3 / 2);
	ASSERT_EQ(string_list.len, 2);

	// Less than decay factor 50%
	string_list.remove(0);
	ASSERT_EQ(string_list._size, (int)1 * 3 / 2);
	ASSERT_EQ(string_list.len, 1);

	// Less than decay factor 50%
	string_list.remove_last();
	ASSERT_EQ(string_list._size, 0);
	ASSERT_EQ(string_list.len, 0);

	EXPECT_DEATH(string_list.remove_last(), "Array is empty!");
}

TEST_F(DynamicArrayTests, Iterator)
{
	const char *string_array[] = {"big", "big", "chungus", "big", "chungus"};
	DynamicArray<const char *> string_list(string_array, 5);

	uint i = 0;
	for (const char *val : string_list)
	{
		EXPECT_STREQ(val, string_array[i]);
		i++;
	}
}
