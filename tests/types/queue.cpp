#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/queue.h>

using namespace Vultr;

TEST(Queue, Push)
{
	Queue<const char *, 3> queue;

	queue.push("Hello");
	queue.push(" my");
	queue.push(" baby");
	ASSERT_FALSE(queue.try_push(" hello").has_value());
}

TEST(Queue, Front)
{
	Queue<const char *> queue;

	queue.push("Hello");
	ASSERT_STRCASEEQ("Hello", queue.front());

	queue.push(" my");
	ASSERT_STRCASEEQ("Hello", queue.front());

	queue.push(" baby");
	ASSERT_STRCASEEQ("Hello", queue.front());
}

TEST(Queue, Pop)
{
	Queue<const char *> queue;
	queue.push("Hello");
	queue.push(" my");
	queue.push(" baby");

	ASSERT_STRCASEEQ("Hello", queue.front());
	queue.pop();
	ASSERT_STRCASEEQ(" my", queue.front());

	queue.push(" hello");
	queue.pop();
	ASSERT_STRCASEEQ(" baby", queue.front());

	queue.pop();
	ASSERT_STRCASEEQ(" hello", queue.front());
}

TEST(Queue, Empty)
{
	Queue<const char *> queue;
	ASSERT_TRUE(queue.empty());
	queue.push("Hello");
	ASSERT_FALSE(queue.empty());
}
