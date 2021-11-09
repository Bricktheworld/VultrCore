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
    static void set_mb_black(MemoryBlock *block) { block->size &= ~(1UL << COLOR_BIT); }
    static void set_mb_red(MemoryBlock *block) { block->size |= 1UL << COLOR_BIT; }
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

    static MemoryBlock *get_left(MemoryBlock *block) { return block->free.left; }
    static MemoryBlock *get_right(MemoryBlock *block) { return block->free.right; }
    static MemoryBlock *get_center(MemoryBlock *block) { return block->free.center; }
    static void assign_right(MemoryBlock *dest, MemoryBlock *src) { dest->free.right = src; }
    static void assign_left(MemoryBlock *dest, MemoryBlock *src) { dest->free.left = src; }
    static void assign_center(MemoryBlock *dest, MemoryBlock *src) { dest->free.center = src; }
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
    static MemoryBlock *rbt_insert(MemoryBlock *h, MemoryBlock *n);
    static void insert_free_mb(MemoryBlock *block, MemoryArena *arena)
    {
        ASSERT_MB_INITIALIZED(block);
        ASSERT_MB_FREE(block);
        arena->free_root = rbt_insert(arena->free_root, block);
        ASSERT(arena->free_root != nullptr, "Something went wrong inserting memory block!");
        set_mb_black(arena->free_root);
    }
    static MemoryBlock *rbt_delete(MemoryBlock *h, MemoryBlock *n);
    static void remove_free_mb(MemoryBlock *block, MemoryArena *arena)
    {
        ASSERT_MB_INITIALIZED(block);
        ASSERT_MB_FREE(block);
        arena->free_root = rbt_delete(arena->free_root, block);
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

    static MemoryBlock *rotate_left(MemoryBlock *h)
    {
        ASSERT_MB_FREE(h);
        auto *x = get_right(h);
        ASSERT_MB_FREE(x);
        assign_right(h, get_left(x));
        assign_left(x, h);
        set_mb_color(x, is_red(h));
        set_mb_red(h);
        return x;
    }
    static MemoryBlock *rotate_right(MemoryBlock *h)
    {
        ASSERT_MB_FREE(h);
        auto *x = get_left(h);
        ASSERT_MB_FREE(x);
        assign_left(h, get_right(x));
        assign_right(x, h);
        set_mb_color(x, is_red(h));
        set_mb_red(h);
        return x;
    }

    static MemoryBlock *move_red_left(MemoryBlock *h)
    {
        color_flip(h);
        auto *r = get_right(h);
        if (is_red(get_left(r)))
        {
            assign_right(h, rotate_right(r));
            h = rotate_left(h);
            color_flip(h);
        }
        return h;
    }

    static MemoryBlock *move_red_right(MemoryBlock *h)
    {
        color_flip(h);
        auto *ll = get_left(get_left(h));
        if (is_red(ll))
        {
            h = rotate_right(h);
            color_flip(h);
        }
        return h;
    }

    static MemoryBlock *rbt_insert(MemoryBlock *h, MemoryBlock *n)
    {
        if (h == nullptr)
        {
            set_mb_red(n);
            return n;
        }

        auto *l     = get_left(h);
        auto *r     = get_right(h);
        auto n_size = get_mb_size(n);
        auto h_size = get_mb_size(h);

        if (is_red(l) && is_red(r))
        {
            color_flip(h);
        }

        if (n_size < h_size)
        {
            assign_left(h, rbt_insert(l, n));
        }
        else if (n_size > h_size)
        {
            assign_right(h, rbt_insert(r, n));
        }
        else if (n_size == h_size)
        {
            ASSERT(h != n, "Attempting to insert memory block into memory arena which already exists.");
            add_center(h, n);
        }

        if (is_red(r) && is_black(l))
        {
            h = rotate_left(h);
        }

        if (is_red(l) && is_red(get_left(l)))
        {
            h = rotate_right(h);
        }

        return h;
    }

    static MemoryBlock *fixup(MemoryBlock *h)
    {
        auto *r = get_right(h);
        auto *l = get_left(h);
        if (is_red(r))
        {
            h = rotate_left(h);
        }

        if (is_red(l) && l != nullptr && is_red(get_left(l)))
        {
            h = rotate_right(h);
        }

        if (is_red(l) && is_red(h))
        {
            color_flip(h);
        }

        return h;
    }

    static MemoryBlock *delete_min(MemoryBlock *h)
    {
        auto *r = get_right(h);
        auto *l = get_left(h);
        if (l == nullptr)
        {
            // TODO(Brandon): Memory leak.
            return nullptr;
        }

        if (is_black(l) && l != nullptr && is_black(get_left(l)))
        {
            h = move_red_left(h);
        }

        assign_left(h, delete_min(l));

        return fixup(h);
    }

    static MemoryBlock *rbt_delete(MemoryBlock *h, MemoryBlock *n)
    {
        auto *l     = get_left(h);
        auto *r     = get_right(h);
        auto n_size = get_mb_size(n);
        auto h_size = get_mb_size(h);

        if (n_size < h_size)
        {
            if (is_black(l) && l != nullptr && is_black(get_left(l)))
            {
                h = move_red_left(h);
            }
            assign_left(h, rbt_delete(l, n));
        }
        else
        {
            if (is_red(l))
            {
                h = rotate_right(h);
            }
            if (n_size == h_size)
            {
                if (n != h)
                {
                    find_remove_center(h, n);
                }
                // ASSERT(n == h, "TODO(Brandon): Handle this case.");
                // TODO(Brandon): This is a memory leak, but I don't care right now.
                // This implementation will be different in the allocator anyway because
                // this will be essentially a bucket containing multiple memory blocks of the same size.
                // All we need to do is remove if it is the same memory address.
                return get_center(h);
            }
            if (is_black(r) && r != nullptr && is_black(get_left(r)))
            {
                h = move_red_right(h);
            }

            if (n_size == h_size)
            {
                NOT_IMPLEMENTED("TODO(Brandon): Figure this shit out.");
                // TODO(Brandon): Figure this shit out.
                // h->data  = min(h)->data;
                // h->right = delete_min(h->right);
            }
            else
            {
                assign_right(h, rbt_delete(r, n));
            }
        }

        return fixup(h);
    }
} // namespace Vultr
