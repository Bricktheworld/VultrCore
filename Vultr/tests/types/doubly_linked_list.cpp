#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/doubly_linked_list.h>

using namespace vtl;
TEST(DoublyLinkedList, New)
{
    auto string_list = new_doubly_linked_list<const char *>();
}

TEST(DoublyLinkedList, PushBack)
{
    auto string_list = new_doubly_linked_list<const char *>();
    doubly_linked_list_push_back(string_list, "Test");
    EXPECT_STREQ(string_list[0], "Test");
}

TEST(DoublyLinkedList, Insert)
{
    auto string_list = new_doubly_linked_list<const char *>();
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Chungus");
    doubly_linked_list_insert(string_list, "urmother", 1);
    EXPECT_STREQ(string_list[0], "Big");
    EXPECT_STREQ(string_list[1], "urmother");
    EXPECT_STREQ(string_list[2], "Big");
    EXPECT_STREQ(string_list[3], "Chungus");
    doubly_linked_list_insert(string_list, "joe", 4);
    EXPECT_STREQ(string_list[4], "joe");
}

TEST(DoublyLinkedList, Iterator)
{
    const char *values[3] = {"Big", "Big", "Chungus"};
    auto string_list = new_doubly_linked_list<const char *>();
    for (int i = 0; i < 3; i++)
    {
        doubly_linked_list_push_back(string_list, values[i]);
    }

    uint i = 0;
    for (auto iterator = string_list.begin(); iterator != string_list.end(); iterator++)
    {
        EXPECT_STREQ((*iterator), values[i]) << "Index " << i;
        i++;
    }
    EXPECT_EQ(i, 3);

    i = 0;
    for (auto val : string_list)
    {
        EXPECT_STREQ(val, values[i]) << "Index " << i;
        i++;
    }
    EXPECT_EQ(i, 3);
    string_list = new_doubly_linked_list<const char *>();
    uint index = 0;
    for (auto element : string_list)
    {
        index++;
    }
    EXPECT_EQ(index, 0);
}

TEST(DoublyLinkedList, Exists)
{
    auto string_list = new_doubly_linked_list<const char *>();
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Chungus");
    EXPECT_TRUE(doubly_linked_list_exists(string_list, "Big"));
    EXPECT_FALSE(doubly_linked_list_exists(string_list, "joe"));
}

TEST(DoublyLinkedList, Find)
{
    auto string_list = new_doubly_linked_list<const char *>();
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Chungus");
    EXPECT_EQ(doubly_linked_list_find(string_list, "Big"), 0);
    EXPECT_EQ(doubly_linked_list_find(string_list, "Chungus"), 2);
    EXPECT_EQ(doubly_linked_list_find(string_list, "joe"), -1);
}

TEST(DoublyLinkedList, Delete)
{
    auto string_list = new_doubly_linked_list<const char *>();
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Chungus");
    doubly_linked_list_delete(string_list, 1);
    EXPECT_STREQ(string_list[0], "Big");
    EXPECT_STREQ(string_list[1], "Chungus");
    EXPECT_NO_THROW(string_list[0]);
    EXPECT_NO_THROW(string_list[1]);
    EXPECT_ANY_THROW(string_list[2]);
    doubly_linked_list_delete(string_list, 0);
    EXPECT_ANY_THROW(string_list[1]);
}

TEST(DoublyLinkedList, Length)
{
    auto string_list = new_doubly_linked_list<const char *>();
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Big");
    doubly_linked_list_push_back(string_list, "Chungus");
    EXPECT_EQ(doubly_linked_list_length(string_list), 3);
}
