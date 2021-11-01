// TODO: Reimplement with new engine
#include <gtest/gtest.h>
#define private public
#define protected public

#include <core/memory/vultr_memory.h>

using namespace Vultr;
// Function to print inorder traversal
// of the fixated tree
static void inorder(Node *root)
{
    if (root == NULL)
        return;
    inorder(root->left);
    printf("%d ", root->data);
    inorder(root->right);
}

void print_rbt(const char *prefix, const Node *node, bool isLeft)
{
    if (node != nullptr)
    {
        printf("%s", prefix);
        printf("%s", isLeft ? "├──" : "└──");
#define RED_PRINT "\x1B[31m"
#define RESET_PRINT "\x1B[0m"

        // print the value of the node
        if (node->color == RED)
        {
            printf(RED_PRINT "%d\n" RESET_PRINT, node->data);
        }
        else
        {
            printf("%d\n", node->data);
        }

        // enter the next tree level - left and right branch
        u32 len = strlen(prefix);
        char new_prefix[len + 5];
        strcpy(new_prefix, prefix);
        strcat(new_prefix, (isLeft ? "│   " : "    "));

        print_rbt(new_prefix, node->left, true);
        print_rbt(new_prefix, node->right, false);
    }
}

void print_rbt(const Node *node)
{
    print_rbt("", node, false);
}

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
