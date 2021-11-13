#include "vultr_memory.h"
#include <types/types.h>

namespace Vultr
{
    // mb = memory block
    // rbt = red-black tree
    // l = left
    // ll = left left
    // r = right
#define RED 1
#define BLACK 0

#define FREE 0
#define ALLOCATED 1

#define INITIALIZED_BIT 0
#define ALLOCATION_BIT 1
#define COLOR_BIT 2
#define LOWEST_3_BITS 0x7
#define HEADER_SIZE (sizeof(MemoryBlock) - sizeof(FreeMemory))
#define ASSERT_MB_INITIALIZED(block) ASSERT(BIT_IS_HIGH(block->size, INITIALIZED_BIT), "Memory block has not been initialized! Please call `init_free_mb` first!")
#define ASSERT_MB_FREE(block) ASSERT(BIT_IS_LOW(block->size, COLOR_BIT), "Memory block is not free!")
#define ASSERT_MB_ALLOCATED(block) ASSERT(BIT_IS_HIGH(block->size, COLOR_BIT), "Memory block has not been allocated!")

    // Bit hack magic to manipulate lowest 3 bits of our size.
    // This is possible because alignment is 8 bytes minimum so the lowest 3 bits (1, 2, 4) will always be rounded to 0.
    // These can then be used to hold our initialization flag, allocation flag, and color bit
    static u64 get_mb_size(MemoryBlock *block) { return block->size & ~LOWEST_3_BITS; }
    static void *get_mb_memory(MemoryBlock *block)
    {
        ASSERT_MB_ALLOCATED(block);
        return block + HEADER_SIZE;
    }
    static void set_mb_allocated(MemoryBlock *block) { block->size |= 1UL << ALLOCATION_BIT; }
    static void set_mb_free(MemoryBlock *block) { block->size &= ~(1UL << ALLOCATION_BIT); }
    static void set_mb_color(MemoryBlock *block, u8 color) { block->size = (block->size & ~(1UL << COLOR_BIT)) | (color << COLOR_BIT); }
    static void set_mb_black(MemoryBlock *block)
    {
        if (block == nullptr)
            return;
        block->size &= ~(1UL << COLOR_BIT);
    }
    static void set_mb_red(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot set color of memory block nullptr");
        block->size |= 1UL << COLOR_BIT;
    }
    static bool is_red(MemoryBlock *block)
    {
        if (block == nullptr)
            return false;
        return BIT_IS_HIGH(block->size, COLOR_BIT);
    }
    static bool is_black(MemoryBlock *block) { return !is_red(block); }
    static void flip_color(MemoryBlock *block)
    {
        if (block == nullptr)
            return;
        block->size ^= (1UL << COLOR_BIT);
    }
    static void color_flip(MemoryBlock *block)
    {
        ASSERT_MB_FREE(block);
        flip_color(block);
        flip_color(block->free.left);
        flip_color(block->free.right);
    }

    static MemoryBlock *get_parent(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot assign get parent from NULL block!");
        return block->free.parent;
    }
    static MemoryBlock *get_grandparent(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot assign get grandparent from NULL block!");
        auto *parent = get_parent(block);
        ASSERT(parent != nullptr, "Block does not have a parent and thus doesn't have a grandparent!");
        return get_parent(parent);
    }
    static MemoryBlock *get_left(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot assign get left from NULL block!");
        return block->free.left;
    }
    static MemoryBlock *get_right(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot assign get right from NULL block!");
        return block->free.right;
    }
    static MemoryBlock *get_center(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot assign get center from NULL block!");
        return block->free.center;
    }
    static void assign_parent(MemoryBlock *dest, MemoryBlock *src)
    {
        ASSERT(dest != nullptr, "Cannot assign to NULL destination!");
        dest->free.parent = src;
    }
    static void assign_right(MemoryBlock *dest, MemoryBlock *src)
    {
        ASSERT(dest != nullptr, "Cannot assign to NULL destination!");
        dest->free.right = src;
        if (src != nullptr)
        {
            assign_parent(src, dest);
        }
    }
    static void assign_left(MemoryBlock *dest, MemoryBlock *src)
    {
        ASSERT(dest != nullptr, "Cannot assign to NULL destination!");
        dest->free.left = src;
        if (src != nullptr)
        {
            assign_parent(src, dest);
        }
    }
    static void assign_center(MemoryBlock *dest, MemoryBlock *src)
    {
        ASSERT(dest != nullptr, "Cannot assign to NULL destination!");
        dest->free.center = src;
        if (src != nullptr)
        {
            assign_parent(src, get_parent(dest));
        }
    }
    static void add_center(MemoryBlock *dest, MemoryBlock *src)
    {
        auto *c = get_center(dest);
        if (c == nullptr)
        {
            assign_center(dest, src);
        }
        else
        {
            // Pre-pend to the linked list.
            assign_center(src, c);
            assign_center(dest, src);
        }
    }
    static MemoryBlock *remove_center(MemoryBlock *block)
    {
        auto *c = get_center(block);
        ASSERT(c != nullptr, "Memory block does not have a center!");

        auto *cc = get_center(c);
        assign_center(block, cc);
        assign_center(c, nullptr);
        return c;
    }
    static void find_remove_center(MemoryBlock *block, MemoryBlock *find)
    {
        ASSERT(find != nullptr, "Cannot find a memory block that is NULL.");

        auto *c = get_center(block);
        if (c == nullptr)
        {
            return;
        }
        else if (c == find)
        {
            auto *cc = get_center(c);
            assign_center(block, cc);
            assign_center(c, nullptr);
            return;
        }
        else if (c != find)
        {
            find_remove_center(c, find);
        }
    }
    static void insert_free_mb(MemoryBlock *block, MemoryArena *arena)
    {
        ASSERT_MB_INITIALIZED(block);
        ASSERT_MB_FREE(block);
        rbt_insert(arena, block);
        ASSERT(arena->free_root != nullptr, "Something went wrong inserting memory block!");
        set_mb_black(arena->free_root);
        assign_parent(arena->free_root, nullptr);
    }
    static MemoryBlock *rbt_delete(MemoryArena *arena, MemoryBlock *n);
    static void remove_free_mb(MemoryBlock *block, MemoryArena *arena)
    {
        ASSERT_MB_INITIALIZED(block);
        ASSERT_MB_FREE(block);
        NOT_IMPLEMENTED("Reimplementing RN.");
        // rbt_delete(arena, block);
        if (arena->free_root != nullptr)
        {
            set_mb_black(arena->free_root);
        }
    }

    static MemoryBlock *mb_best_match(MemoryBlock *h, u64 size)
    {
        // If h is a leaf, then we can assume it is the closest block to the size we need so we can just split it

        if (h == nullptr)
            return nullptr;

        if (size < get_mb_size(h))
        {
            auto *l = get_left(h);
            // If there is no block that exists of a smaller size, then we should just return the current node, `h` and we will split that later.
            if (l == nullptr)
                return h;

            return mb_best_match(l, size);
        }
        else if (size > get_mb_size(h))
        {
            auto *r = get_right(h);

            // If there is no block that exists of a larger size, then the requested allocation size is impossible. Thus we must return nullptr.
            if (r == nullptr)
                return nullptr;

            return mb_best_match(r, size);
        }
        else // if(size == get_mb_size(h))
        {
            return h;
        }
    }

    static u32 get_height(MemoryBlock *h)
    {
        if (h == nullptr)
        {
            return 0;
        }
        else
        {
            auto *l = get_left(h);
            auto *r = get_right(h);
            return 1 + MAX(get_height(l), get_height(r));
        }
    }
    static MemoryBlock *min(MemoryBlock *h)
    {
        auto *l = get_left(h);
        if (l != nullptr)
        {
            return min(l);
        }
        else
        {
            return h;
        }
    }

    static void init_free_mb(MemoryBlock *block, u64 size, MemoryBlock *prev, MemoryBlock *next)
    {
        block->size        = (~(1UL << ALLOCATION_BIT) & size) | (1UL << INITIALIZED_BIT);
        block->next        = next;
        block->prev        = prev;
        block->free.parent = nullptr;
        block->free.center = nullptr;
        block->free.left   = nullptr;
        block->free.right  = nullptr;
    }

    MemoryArena *init_mem_arena(u64 size, u8 alignment)
    {
        ASSERT(size > sizeof(MemoryBlock), "Why are you initializing a memory arena that is literally smaller than 48 bytes...");
        // NOTE(Brandon): These should really be the only two places where malloc and free are ever called throughout the lifetime of the program.
        // Every other dynamic allocation should be done through the memory arenas.

        // TODO(Brandon): Replace malloc with a more performant, platform specific, method.
        auto *arena = static_cast<MemoryArena *>(malloc(sizeof(MemoryArena)));

        if (arena == nullptr)
        {
            return nullptr;
        }
        arena->memory = static_cast<MemoryBlock *>(malloc(size));
        if (arena->memory == nullptr)
        {
            free(arena);
            return nullptr;
        }

        MemoryBlock *h = arena->memory;

        // Subtract the size of the memory header because this will exist at all times
        init_free_mb(h, size - HEADER_SIZE, nullptr, nullptr);
        insert_free_mb(h, arena);

        arena->alignment = alignment;

        return arena;
    }

    static MemoryBlock *split_mb(MemoryBlock *b, u64 new_size)
    {
        // If the new size is exactly the same size as our memory block, there is no reason to split.
        if (new_size == get_mb_size(b))
        {
            return nullptr;
        }
        u32 lowest_bits = b->size & LOWEST_3_BITS;
        u32 old_size    = get_mb_size(b) + HEADER_SIZE;
        b->size         = new_size | lowest_bits;

        MemoryBlock *new_block = reinterpret_cast<MemoryBlock *>(reinterpret_cast<char *>(b) + new_size + HEADER_SIZE);
        init_free_mb(new_block, old_size - new_size - HEADER_SIZE, b, b->next);

        b->next = new_block;
        return new_block;
    }
    void *mem_arena_alloc(MemoryArena *arena, u64 size)
    {
        // Find a memory block of suitable size.
        auto *best_match = mb_best_match(arena->free_root, size);
        PRODUCTION_ASSERT(best_match != nullptr, "Not enough memory to allocate!");
        ASSERT(get_mb_size(best_match) >= size, "");

        // Delete this memory block from the red black tree.
        remove_free_mb(best_match, arena);

        // If need be, split the memory block into the size that we need.
        auto *new_block = split_mb(best_match, size);
        if (new_block != nullptr)
        {
            // Insert this new memory block as free into the memory arena.
            insert_free_mb(new_block, arena);
        }

        // Set our memory block to allocated.
        set_mb_allocated(best_match);
        return reinterpret_cast<char *>(best_match) + HEADER_SIZE;
    }
    void mem_arena_free(MemoryArena *arena, void *data)
    {
        MemoryBlock *block_to_free = reinterpret_cast<MemoryBlock *>(reinterpret_cast<char *>(data) - HEADER_SIZE);
        u64 size                   = get_mb_size(block_to_free);
        auto *prev                 = block_to_free->prev;
        auto *next                 = block_to_free->next;
        init_free_mb(block_to_free, size, prev, next);
        insert_free_mb(block_to_free, arena);
    }
    void destroy_mem_arena(MemoryArena *arena)
    {
        ASSERT(arena != nullptr && arena->memory != nullptr, "Invalid memory arena!");
        free(arena->memory);
        free(arena);
    }

    // Red-black tree rules:
    // - Every node is either black or red.
    // - The root node is always black.
    // - Red nodes cannot have red children (no two red nodes can exist in a row).
    // - Every path from a node to any of its descendants null nodes have the same number of black nodes (All nodes have the same black height).
    //
    // Black Height: The number of black nodes on a path to a leaf node. Leaf nodes are black nodes because they are null.
    // A red black tree of height h has black height >= h/2
    // The height of a red black tree with n nodes is h <= 2 logBase(2, n+1)
    // All leaves (null) are black.
    // The black depth of a node is defined as the number of black ancestor nodes.
    //
    //

    static MemoryBlock *bst_insert(MemoryBlock *h, MemoryBlock *n)
    {
        if (h == nullptr)
        {
            set_mb_red(n);
            return n;
        }

        u64 h_size = get_mb_size(h);
        u64 n_size = get_mb_size(n);

        if (n_size < h_size)
        {
            assign_left(h, bst_insert(get_left(h), n));
        }
        else if (n_size > h_size)
        {
            assign_right(h, bst_insert(get_right(h), n));
        }
        else if (n_size == h_size)
        {
            add_center(h, n);
        }

        return h;
    }

    static void rbt_right_rotate(MemoryArena *arena, MemoryBlock *n)
    {
        auto *l = get_left(n);
        assign_left(n, get_right(l));

        assign_parent(l, get_parent(n));
        // If node n is not the root...
        if (get_parent(n) != nullptr)
        {
            // If n is a left child...
            if (n == get_left(get_parent(n)))
            {
                assign_left(get_parent(n), l);
            }
            // If n is a right child...
            else
            {
                assign_right(get_parent(n), l);
            }
        }
        // If it is, then we need to fix the root node pointed to in memory arena.
        else
        {
            arena->free_root = l;
        }

        assign_right(l, n);
    }

    static void rbt_left_rotate(MemoryArena *arena, MemoryBlock *n)
    {
        auto *r = get_right(n);
        assign_right(n, get_left(r));

        assign_parent(r, get_parent(n));
        // If node n is not the root...
        if (get_parent(n) != nullptr)
        {
            // If n is a left child...
            if (n == get_left(get_parent(n)))
            {
                assign_left(get_parent(n), r);
            }
            // If n is a right child...
            else
            {
                assign_right(get_parent(n), r);
            }
        }
        // If it is, then we need to fix the root node pointed to in memory arena.
        else
        {
            arena->free_root = r;
        }
        assign_left(r, n);
    }

    void rbt_insert(MemoryArena *arena, MemoryBlock *n)
    {
        arena->free_root = bst_insert(arena->free_root, n);

        MemoryBlock *parent      = nullptr;
        MemoryBlock *grandparent = nullptr;

        // Traverse until either:
        // - We reach the root node.
        // - The current node is black.
        // - The current node's parent is black.

        while (n != arena->free_root && is_red(n) && is_red(get_parent(n)))
        {
            parent = get_parent(n);
            // NOTE(Brandon): grandparent will never be nullptr because in order for it to be nullptr parent would have to be the root, and that is simply not allowed in the loop because the root is always black and
            // the parent must be red.
            grandparent = get_grandparent(n);

            // Left variant of checks
            if (parent == get_left(grandparent))
            {
                auto *uncle = get_right(grandparent);

                // If uncle is also red...
                if (is_red(uncle))
                {
                    set_mb_red(grandparent);
                    set_mb_black(parent);
                    set_mb_black(uncle);
                    n = grandparent;
                }
                else
                {
                    // If n is a right child of parent...
                    if (n == get_right(parent))
                    {
                        rbt_left_rotate(arena, parent);
                        n      = parent;
                        parent = get_parent(n);
                    }

                    // If n is a left child of its parent...
                    rbt_right_rotate(arena, grandparent);
                    auto c = is_red(parent);
                    set_mb_color(parent, is_red(grandparent));
                    set_mb_color(grandparent, c);
                    n = parent;
                }
            }
            // Right variant of checks
            else
            {
                auto *uncle = get_left(grandparent);

                // If uncle is also red...
                if (is_red(uncle))
                {
                    set_mb_red(grandparent);
                    set_mb_black(parent);
                    set_mb_black(uncle);
                    n = grandparent;
                }
                else
                {
                    // If n is a left child of parent...
                    if (n == get_left(parent))
                    {
                        rbt_right_rotate(arena, parent);
                        n      = parent;
                        parent = get_parent(n);
                    }

                    // If n is a right child of its parent...
                    rbt_left_rotate(arena, grandparent);
                    auto c = is_red(parent);
                    set_mb_color(parent, is_red(grandparent));
                    set_mb_color(grandparent, c);
                    n = parent;
                }
            }
        }

        set_mb_black(arena->free_root);
    }

    static MemoryBlock *rbt_delete(MemoryArena *arena, MemoryBlock *n)
    {
        // TODO(Brandon): Implement.
        NOT_IMPLEMENTED("Not implemented");
    }
} // namespace Vultr
