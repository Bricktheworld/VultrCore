#include <gtest/gtest.h>
#include <render/types/material.h>
#include <filesystem/importers/shader_importer.h>
#include "gui/basic_rendering_test.h"

int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
