#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/types.h>

TEST(String, Length)
{
    str_len("Some thing");
}
