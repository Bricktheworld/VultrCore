#include <gtest/gtest.h>

#include <platform/platform.h>

using namespace Vultr;
TEST(Time, CurrentMs)
{
	u64 current_ms = Platform::current_ms();
	ASSERT_NE(current_ms, 0);
}
