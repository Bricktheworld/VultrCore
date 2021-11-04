// TODO: Reimplement with new engine
#include <gtest/gtest.h>
#define private public
#define protected public

#include <core/memory/vultr_memory.h>

using namespace Vultr;
TEST(VultrMemory, RedBlackTree)
{
    RBTree tree;

    for (u32 i = 0; i < 15; i++)
    {
        auto *node = new Node();
        node->data = i;
        rbt_insert(&tree, node);
    }
}
