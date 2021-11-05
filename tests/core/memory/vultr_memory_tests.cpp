#include <gtest/gtest.h>
#define private public
#define protected public

#include <core/memory/vultr_memory.h>
#include <stdlib.h>
#include <math.h>

using namespace Vultr;
static void print_bt(const std::string &prefix, const Node *node, bool isLeft)
{
    if (node != nullptr)
    {
        std::cout << prefix;

        if (node->color == RED)
        {
            std::cout << "\033[0;31m";
        }

        std::cout << (isLeft ? "├──" : "└──");

        // print the value of the node
        std::cout << node->data << std::endl;

        if (node->color == RED)
        {

            std::cout << "\033[0m";
        }

        // enter the next tree level - left and right branch
        print_bt(prefix + (isLeft ? "│   " : "    "), node->left, true);
        print_bt(prefix + (isLeft ? "│   " : "    "), node->right, false);
    }
}

static void print_bt(const Node *node)
{
    print_bt("", node, false);
}

static void fill_tree(RBTree *tree, Node *nodes, u32 len)
{
    for (u32 i = 0; i < len; i++)
    {
        auto *node = &nodes[i];
        node->left = nullptr;
        node->right = nullptr;
        node->parent = nullptr;
        node->color = RED;
        rbt_insert(tree, node);
        EXPECT_TRUE(rbt_search(tree, node->data) != nullptr);
    }
}

TEST(VultrMemory, Insert)
{
    Node nodes[] = {{5}, {1}, {3}, {6}, {9}, {2}};
    u32 len = sizeof(nodes) / sizeof(Node);

    RBTree tree;
    fill_tree(&tree, nodes, len);
}

TEST(VultrMemory, Delete)
{
    Node nodes[] = {{5}, {1}, {3}, {6}, {9}, {2}};
    u32 len = sizeof(nodes) / sizeof(Node);

    RBTree tree;
    fill_tree(&tree, nodes, len);

    u32 index = 0;
    rbt_delete(&tree, &nodes[index]);
    len--;
    EXPECT_TRUE(rbt_search(&tree, nodes[0].data) == nullptr);
}

TEST(VultrMemory, Search)
{
    Node nodes[] = {{5}, {1}, {3}, {6}, {9}, {2}};
    u32 len = sizeof(nodes) / sizeof(Node);

    RBTree tree;
    fill_tree(&tree, nodes, len);

    u32 index = 3;
    auto *search = rbt_search(&tree, nodes[index].data);
    ASSERT_FALSE(search == nullptr);
    ASSERT_EQ(search, &nodes[index]);
}

TEST(VultrMemory, Height)
{
    for (s32 i = 1; i < 2000; i++)
    {
        RBTree tree;
        Node nodes[i];
        for (s32 j = 0; j < i; j++)
        {
            nodes[j].data = rand() % i;
        }
        fill_tree(&tree, nodes, i);

        ASSERT_TRUE(rbt_height(&tree) <= (u32)(2 * ceil(log((f64)i) / log(2) + 1)));
    }
}
