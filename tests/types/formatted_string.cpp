#include <gtest/gtest.h>
#include <types/formatted_string.h>

using namespace Vultr;
TEST(FormattedString, Test) { FormattedString("{{}} Test", 1); }