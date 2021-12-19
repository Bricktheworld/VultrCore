#include <gtest/gtest.h>
#include <render/types/material.h>
#include <sys/resource.h>
#include <filesystem/importers/shader_importer.h>
#include "gui/basic_rendering_test.h"

int main(int argc, char **argv)
{
    // basic_rendering_test();
    struct rlimit core_limits;
    core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &core_limits);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
    using namespace Vultr;
    Material material = {};
    fprintf(stdout, "%lu", sizeof(material));
}
