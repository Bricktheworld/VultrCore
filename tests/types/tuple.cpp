#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/tuple.h>

TEST(Tuple, Constructor)
{
	Vultr::Tuple<u32, f64, const char *> some_tuple(0, 1.0, "Some string");
	some_tuple.apply<void>([](u32 integer, f64 floating, const char *some_string) {
		ASSERT_EQ(integer, 0);
		ASSERT_DOUBLE_EQ(floating, 1.0);
	});
	ASSERT_EQ(some_tuple.get<0>(), 0);
	ASSERT_EQ(some_tuple.get<1>(), 1.0);
	ASSERT_STRCASEEQ(some_tuple.get<2>(), "Some string");

	ASSERT_EQ(some_tuple.get<u32>(), 0);
	ASSERT_EQ(some_tuple.get<f64>(), 1.0);
	ASSERT_STRCASEEQ(some_tuple.get<const char *>(), "Some string");
}
