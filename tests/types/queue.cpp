// TODO(Brandon): Readd tests when custom allocator is implemented into types.
// #include <gtest/gtest.h>
// #define private public
// #define protected public
// #include <types/queue.h>

// using namespace vtl;

// TEST(Queue, Initialize)
// {
//     Queue<const char *, 15> queue;
//     ASSERT_EQ(queue._size, 15);
//     ASSERT_EQ(queue.len, 0);
//     ASSERT_NE(queue._array, nullptr);
// }

// TEST(Queue, Push)
// {
//     Queue<const char *, 2, 3, 2> queue;

//     queue.push("Hello");
//     ASSERT_EQ(queue._size, 2);
//     ASSERT_EQ(queue.len, 1);

//     queue.push(" my");
//     ASSERT_EQ(queue._size, 2);
//     ASSERT_EQ(queue.len, 2);

//     queue.push(" baby");
//     ASSERT_EQ(queue._size, 4);
//     ASSERT_EQ(queue.len, 3);
// }

// TEST(Queue, Front)
// {
//     Queue<const char *, 2, 3, 2> queue;

//     queue.push("Hello");
//     ASSERT_EQ(queue._size, 2);
//     ASSERT_EQ(queue.len, 1);

//     ASSERT_STRCASEEQ("Hello", *queue.front());

//     queue.push(" my");
//     ASSERT_EQ(queue._size, 2);
//     ASSERT_EQ(queue.len, 2);

//     ASSERT_STRCASEEQ("Hello", *queue.front());

//     queue.push(" baby");
//     ASSERT_EQ(queue._size, 4);
//     ASSERT_EQ(queue.len, 3);

//     ASSERT_STRCASEEQ("Hello", *queue.front());
// }

// TEST(Queue, Pop)
// {
//     Queue<const char *, 2, 3, 2, 26> queue;
//     queue.push("Hello");
//     queue.push(" my");
//     queue.push(" baby");

//     ASSERT_EQ(queue._size, 4);
//     ASSERT_EQ(queue.len, 3);
//     ASSERT_STRCASEEQ("Hello", *queue.front());
//     queue.pop();
//     ASSERT_EQ(queue._size, 4);
//     ASSERT_EQ(queue.len, 2);
//     ASSERT_STRCASEEQ(" my", *queue.front());

//     queue.push(" hello");
//     ASSERT_EQ(queue._size, 4);
//     ASSERT_EQ(queue.len, 3);

//     queue.pop();
//     ASSERT_EQ(queue._size, 4);
//     ASSERT_EQ(queue.len, 2);
//     ASSERT_STRCASEEQ(" baby", *queue.front());

//     queue.pop();
//     ASSERT_EQ(queue._size, 2);
//     ASSERT_EQ(queue.len, 1);
//     ASSERT_STRCASEEQ(" hello", *queue.front());
// }

// TEST(Queue, Empty)
// {
//     Queue<const char *> queue;
//     ASSERT_TRUE(queue.empty());
//     queue.push("Hello");
//     ASSERT_FALSE(queue.empty());
// }
