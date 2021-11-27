#include <gtest/gtest.h>
#define private public
#define protected public

#include <platform/platform.h>
#include <platform/platform_imp.h>

using namespace Vultr;

TEST(ThreadTests, NewThread)
{
    bool some_value = true;
    Platform::new_thread(
        nullptr, nullptr,
        [](bool some_arg) {
            if (some_arg)
                printf("Hello world!");
        },
        some_value);
}
