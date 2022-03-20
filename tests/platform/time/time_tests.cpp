#include <gtest/gtest.h>

#include <platform/platform.h>

using namespace Vultr;
TEST(Time, CurrentMs) { printf("Current MS since epoch %lu", Platform::current_ms()); }
