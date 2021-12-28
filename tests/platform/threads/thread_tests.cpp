#include <gtest/gtest.h>
#define private public
#define protected public

#include <platform/platform.h>

using namespace Vultr;

static bool delegate(bool some_arg)
{
	if (some_arg)
		return true;
	else
		return false;
}

TEST(ThreadTests, NewThread)
{
	bool some_value   = true;
	bool return_value = false;

	Platform::ThreadArgs thread_args(delegate, &return_value, some_value);
	auto thread = Platform::new_thread(&thread_args);
	Platform::join_thread(&thread);
	ASSERT_TRUE(return_value);

	return_value = false;

	Platform::jthread(delegate, &return_value, some_value);
	ASSERT_TRUE(return_value);
}

TEST(ThreadTests, MutexLock)
{
	Platform::Mutex mutex;
	Platform::mutex_lock(&mutex);
	Platform::mutex_unlock(&mutex);
}
