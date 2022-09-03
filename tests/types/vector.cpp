#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/vector.h>
#include <vultr_memory.h>

using namespace Vultr;
TEST(VectorTests, EmptyInitialize)
{
	Vector<const char *, 15> string_list;
	ASSERT_EQ(string_list.capacity(), 15);
	ASSERT_EQ(string_list.size(), 0);
}

TEST(VectorTests, ArrayInitialize)
{
	const char *string_array[] = {"hello", "world"};
	Vector<const char *, 10> string_list(string_array, 2);
	ASSERT_EQ(string_list.capacity(), 10);
	ASSERT_EQ(string_list.size(), 2);
}

TEST(VectorTests, Operator)
{
	const char *string_array[] = {"hello"};
	Vector<const char *> string_list(string_array, 1);
	ASSERT_EQ("hello", string_list[0]);
	string_list[0] = "world";
	ASSERT_EQ("world", string_list[0]);
}

TEST(VectorTests, PushBack)
{
	const char *string_array[] = {"hello"};
	Vector<const char *, 10> string_list(string_array, 1);

	string_list.push_back("world");
	ASSERT_EQ(string_list[1], "world");
	ASSERT_EQ(string_list.capacity(), 10);
	ASSERT_EQ(string_list.size(), 2);
}

TEST(VectorTests, Insert)
{
	const char *string_array[] = {"big", "big", "chungus", "big", "chungus"};
	Vector<const char *, 10> string_list(string_array, 5);

	// Insert middle
	string_list.insert(1, "joe");
	ASSERT_EQ(string_list[0], "big");
	ASSERT_EQ(string_list[1], "joe");
	ASSERT_EQ(string_list[2], "big");
	ASSERT_EQ(string_list[3], "chungus");
	ASSERT_EQ(string_list[4], "big");
	ASSERT_EQ(string_list[5], "chungus");
	ASSERT_EQ(string_list.capacity(), 10);
	ASSERT_EQ(string_list.size(), 6);

	// Insert beginning
	string_list.insert(0, "mama");
	ASSERT_EQ(string_list[0], "mama");
	ASSERT_EQ(string_list[1], "big");
	ASSERT_EQ(string_list[2], "joe");
	ASSERT_EQ(string_list[3], "big");
	ASSERT_EQ(string_list[4], "chungus");
	ASSERT_EQ(string_list[5], "big");
	ASSERT_EQ(string_list[6], "chungus");
	ASSERT_EQ(string_list.capacity(), 10);
	ASSERT_EQ(string_list.size(), 7);

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
	ASSERT_EQ(string_list.capacity(), 10);
	ASSERT_EQ(string_list.size(), 8);

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
	ASSERT_EQ(string_list.capacity(), (int)(11 * 3 / 2));
	ASSERT_EQ(string_list.size(), 11);
}

// TEST_F(VectorTests, Delete)
//{
//	const char *string_array[] = {"big", "big", "chungus", "big", "chungus"};
//
//	const u32 decay_percentage = 50;
//	Vector<const char *, 0> string_list(string_array, 5);
//
//	ASSERT_EQ(string_list._capacity, (int)5 * 3 / 2);
//	ASSERT_EQ(string_list._length, 5);
//
//	string_list.remove(1);
//	ASSERT_EQ(string_list[0], "big");
//	ASSERT_EQ(string_list[1], "chungus");
//	ASSERT_EQ(string_list[2], "big");
//	ASSERT_EQ(string_list[3], "chungus");
//	ASSERT_EQ(string_list._capacity, (int)5 * 3 / 2);
//	ASSERT_EQ(string_list._length, 4);
//
//	// Less than decay factor 50%
//	string_list.remove(3);
//	ASSERT_EQ(string_list[0], "big");
//	ASSERT_EQ(string_list[1], "chungus");
//	ASSERT_EQ(string_list[2], "big");
//
//	ASSERT_EQ(string_list._capacity, (int)3 * 3 / 2);
//	ASSERT_EQ(string_list._length, 3);
//
//	string_list.remove(0);
//	ASSERT_EQ(string_list[0], "chungus");
//	ASSERT_EQ(string_list[1], "big");
//	ASSERT_EQ(string_list._capacity, (int)3 * 3 / 2);
//	ASSERT_EQ(string_list._length, 2);
//
//	// Less than decay factor 50%
//	string_list.remove(0);
//	ASSERT_EQ(string_list._capacity, (int)1 * 3 / 2);
//	ASSERT_EQ(string_list._length, 1);
//
//	// Less than decay factor 50%
//	string_list.remove_last();
//	ASSERT_EQ(string_list._capacity, 0);
//	ASSERT_EQ(string_list._length, 0);
// }

TEST(VectorTests, Iterator)
{
	const char *string_array[] = {"big", "big", "chungus", "big", "chungus"};
	Vector<const char *> string_list(string_array, 5);

	uint i = 0;
	for (const char *val : string_list)
	{
		EXPECT_STREQ(val, string_array[i]);
		i++;
	}
}
