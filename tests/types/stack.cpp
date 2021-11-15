// TODO(Brandon): Readd tests when custom allocator is implemented into types.
// #include <gtest/gtest.h>
// #define private public
// #define protected public
// #include <types/stack.h>

// using namespace vtl;

// TEST(Stack, Initialize)
// {
//     Stack<const char *, 15> stack;
//     ASSERT_EQ(stack._size, 15);
//     ASSERT_EQ(stack.len, 0);
//     ASSERT_NE(stack._array, nullptr);
// }

// TEST(Stack, Push)
// {
//     Stack<const char *, 2, 3, 2> stack;

//     stack.push("Hello");
//     ASSERT_EQ(stack._size, 2);
//     ASSERT_EQ(stack.len, 1);

//     stack.push(" my");
//     ASSERT_EQ(stack._size, 2);
//     ASSERT_EQ(stack.len, 2);

//     stack.push(" baby");
//     ASSERT_EQ(stack._size, 4);
//     ASSERT_EQ(stack.len, 3);
// }

// TEST(Stack, Front)
// {
//     Stack<const char *, 2, 3, 2> stack;

//     stack.push("Hello");
//     ASSERT_EQ(stack._size, 2);
//     ASSERT_EQ(stack.len, 1);

//     ASSERT_STRCASEEQ("Hello", *stack.top());

//     stack.push(" my");
//     ASSERT_EQ(stack._size, 2);
//     ASSERT_EQ(stack.len, 2);

//     ASSERT_STRCASEEQ(" my", *stack.top());

//     stack.push(" baby");
//     ASSERT_EQ(stack._size, 4);
//     ASSERT_EQ(stack.len, 3);

//     ASSERT_STRCASEEQ(" baby", *stack.top());
// }

// TEST(Stack, Pop)
// {
//     Stack<const char *, 2, 3, 2, 26> stack;
//     stack.push("Hello");
//     stack.push(" my");
//     stack.push(" baby");

//     ASSERT_EQ(stack._size, 4);
//     ASSERT_EQ(stack.len, 3);
//     ASSERT_STRCASEEQ(" baby", *stack.top());
//     stack.pop();
//     ASSERT_EQ(stack._size, 4);
//     ASSERT_EQ(stack.len, 2);
//     ASSERT_STRCASEEQ(" my", *stack.top());

//     stack.push(" hello");
//     ASSERT_EQ(stack._size, 4);
//     ASSERT_EQ(stack.len, 3);

//     stack.pop();
//     ASSERT_EQ(stack._size, 4);
//     ASSERT_EQ(stack.len, 2);
//     ASSERT_STRCASEEQ(" my", *stack.top());

//     stack.pop();
//     ASSERT_EQ(stack._size, 2);
//     ASSERT_EQ(stack.len, 1);
//     ASSERT_STRCASEEQ("Hello", *stack.top());
// }

// TEST(Stack, Empty)
// {
//     Stack<const char *> stack;
//     ASSERT_TRUE(stack.empty());
//     stack.push("Hello");
//     ASSERT_FALSE(stack.empty());
// }
