#include "vultr_memory.h"

namespace Vultr
{
    MemoryArena *alloc_mem_arena(u64 size, u8 alignment)
    {
        // NOTE(Brandon): These should really be the only two places where malloc and free are ever called throughout the lifetime of the program.
        // Every other dynamic allocation should be done through the memory arenas.

        // TODO(Brandon): Replace malloc with a more performant, platform specific, method
        auto *mem = static_cast<MemoryArena *>(malloc(sizeof(MemoryArena)));

        if (mem == nullptr)
        {
            return nullptr;
        }
        mem->_memory_chunk = malloc(size);
        mem->alignment = alignment;

        MemoryBlock head;
        head.allocated = false;
        head.data = mem->_memory_chunk;

        // Subtract the size of the memory header because this will exist at all times
        head.size = size - sizeof(MemoryHeader);

        mem->head = head;

        return mem;
    }

    static Node *bst_insert(Node *root, Node *n)
    {
        if (root == nullptr)
            return n;

        if (n->data < root->data)
        {
            root->left = bst_insert(root->left, n);
            root->left->parent = root;
        }
        else if (n->data > root->data)
        {
            root->right = bst_insert(root->right, n);
            root->right->parent = root;
        }

        return root;
    }

    static void rbt_left_rotate(Node **root, Node **n)
    {
        Node *right = (*n)->right;

        (*n)->right = right->left;

        if ((*n)->right != nullptr)
        {
            (*n)->right->parent = *n;
        }

        right->parent = (*n)->parent;

        if ((*n)->parent == nullptr)
        {
            *root = right;
        }
        else if (*n == (*n)->parent->left)
        {
            (*n)->parent->left = right;
        }
        else
        {
            (*n)->parent->right = right;
        }

        right->left = *n;
        (*n)->parent = right;
    }

    static void rbt_right_rotate(Node **root, Node **n)
    {
        Node *left = (*n)->left;

        (*n)->left = left->right;

        if ((*n)->left != nullptr)
        {
            (*n)->left->parent = *n;
        }

        left->parent = (*n)->parent;

        if ((*n)->parent == nullptr)
        {
            *root = left;
        }
        else if (*n == (*n)->parent->left)
        {
            (*n)->parent->left = left;
        }
        else
        {
            (*n)->parent->right = left;
        }

        left->right = *n;
        (*n)->parent = left;
    }

    void rbt_insert(RBTree *tree, Node *n)
    {
        tree->root = bst_insert(tree->root, n);

        Node *parent_node = nullptr;
        Node *grand_parent_node = nullptr;

        while (n != tree->root && n->color != BLACK && n->parent->color == RED)
        {
            parent_node = n->parent;
            grand_parent_node = n->parent->parent;

            // Case 0: parent_node is a left child of grand_parent
            if (parent_node == grand_parent_node->left)
            {
                Node *uncle_node = grand_parent_node->right;

                // Case A: uncle_node is also red. Only recoloring required
                if (uncle_node != nullptr && uncle_node->color == RED)
                {
                    grand_parent_node->color = RED;
                    parent_node->color = BLACK;
                    uncle_node->color = BLACK;
                    n = grand_parent_node;
                }
                else
                {
                    // Case B: n is a right child of parent. Left rotation
                    if (n == parent_node->right)
                    {
                        rbt_left_rotate(&tree->root, &n);
                        n = parent_node;
                        parent_node = n->parent;
                    }

                    // Case C:
                    rbt_right_rotate(&tree->root, &grand_parent_node);
                    auto t = parent_node->color;
                    parent_node->color = grand_parent_node->color;
                    grand_parent_node->color = t;
                    n = parent_node;
                }
            }
            // Case 1: parent_node is a right child of grand_parent
            else
            {
                Node *uncle_node = grand_parent_node->left;

                // Case A: uncle_node is also red. Only recoloring.
                if (uncle_node != nullptr && uncle_node->color == RED)
                {
                    grand_parent_node->color = RED;
                    parent_node->color = BLACK;
                    uncle_node->color = BLACK;
                    n = grand_parent_node;
                }
                else
                {
                    // Case B: n is a left child. Right rotation.
                    if (n == parent_node->left)
                    {
                        rbt_right_rotate(&tree->root, &parent_node);
                        n = parent_node;
                        parent_node = n->parent;
                    }

                    // Case C: n is a right child. Left rotation.
                    rbt_left_rotate(&tree->root, &grand_parent_node);
                    auto t = parent_node->color;
                    parent_node->color = grand_parent_node->color;
                    grand_parent_node->color = t;
                    n = parent_node;
                }
            }
        }
        tree->root->color = BLACK;
    }

    void rbt_remove(RBTree *tree, u32 data)
    {
    }

    void *mem_arena_alloc(MemoryArena *arena, u64 size)
    {
    }

    void mem_arena_free(MemoryArena *arena, void *data)
    {
    }

    void mem_arena_free(MemoryArena *mem)
    {
        free(mem);
    }
} // namespace Vultr
