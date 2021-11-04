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

    // TODO(Brandon): Make sure parent assigning is correct.
    static bool is_red(Node *n)
    {
        if (n == nullptr)
            return false;
        return n->color == RED;
    }

    static bool is_black(Node *n)
    {
        return !is_red(n);
    }

    static void flip_color(Node *n)
    {
        if (n == nullptr)
            return;
        n->color = 1 - n->color;
    }

    static void assign_left(Node *h, Node *n)
    {
        h->left = n;
        if (h->left != nullptr)
        {
            h->left->parent = h;
        }
    }

    static void assign_right(Node *h, Node *n)
    {
        h->right = n;
        if (h->right != nullptr)
        {
            h->right->parent = h;
        }
    }

    static Node *rotate_left(Node *h)
    {
        auto *x = h->right;
        assign_right(h, x->left);
        assign_left(x, h);
        x->color = h->color;
        h->color = RED;
        return x;
    }

    static Node *rotate_right(Node *h)
    {
        auto *x = h->left;
        assign_left(h, x->right);
        assign_right(x, h);
        x->color = h->color;
        h->color = RED;
        return x;
    }

    Node *rbt_insert(Node *h, Node *n);
    void rbt_insert(RBTree *t, Node *n);
    static Node *insert_imp(Node *h, Node *n)
    {
        if (h == nullptr)
        {
            n->color = RED;
            return n;
        }

        if (is_red(h->left) && is_red(h->right))
        {
            flip_color(h);
            flip_color(h->left);
            flip_color(h->right);
        }

        if (n->data < h->data)
        {
            h->left = rbt_insert(h->left, n);
            h->left->parent = h;
        }
        else if (n->data > h->data)
        {
            h->right = rbt_insert(h->right, n);
            h->right->parent = h;
        }

        if (is_red(h->right) && is_black(h->left))
        {
            h = rotate_left(h);
        }

        if (is_red(h->left) && is_red(h->left->left))
        {
            h = rotate_right(h);
        }

        return h;
    }

    Node *rbt_insert(Node *h, Node *n)
    {
        h = insert_imp(h, n);
        return h;
    }

    void rbt_insert(RBTree *t, Node *n)
    {
        t->root = rbt_insert(t->root, n);
        t->root->color = BLACK;
        t->root->parent = nullptr;
    }

    Node *rbt_delete(Node *h, Node *n);
    void rbt_delete(RBTree *t, Node *n);
    static Node *delete_imp(Node *h, Node *n)
    {
    }

    Node *rbt_delete(Node *h, Node *n)
    {
        h = delete_imp(h, n);
        return h;
    }

    void rbt_delete(RBTree *t, Node *n)
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
