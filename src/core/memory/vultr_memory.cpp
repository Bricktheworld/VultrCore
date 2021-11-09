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

    // Red-black tree algorithm credit to https://github.com/xieqing/red-black-tree
    static void rbt_rotate_left(MemoryArena *arena, MemoryBlock *x)
    {
        MemoryBlock *y;

        y = get_right(x);

        assign_right(x, get_left(y));
        if (get_right(x) != nullptr)
        {
            assign_parent(get_right(x), x);
        }

        assign_parent(y, get_parent(x));
        if (x == get_left(get_parent(x)))
        {
            assign_left(get_parent(x), y);
        }
        else
        {
            assign_right(get_parent(x), y);
        }

        assign_left(y, x);
        assign_parent(x, y);
    }

    static void rbt_rotate_right(MemoryArena *arena, MemoryBlock *x)
    {
        MemoryBlock *y;

        y = get_left(x);

        assign_left(x, get_right(y));
        if (get_left(x) != nullptr)
        {
            assign_parent(get_left(x), x);
        }

        assign_parent(y, get_parent(x));
        if (x == get_left(get_parent(x)))
        {
            assign_left(get_parent(x), y);
        }
        else
        {
            assign_right(get_parent(x), y);
        }

        assign_right(y, x);
        assign_parent(x, y);
    }

    static void rbt_insert_repair(MemoryArena *arena, MemoryBlock *current)
    {
        MemoryBlock *uncle;
        //
        do
        {
            // Current node is red and parent node is also red
            if (get_parent(current) == get_left(get_grandparent(current)))
            {
                uncle = get_right(get_grandparent(current));
                if (is_red(uncle))
                {
                    // Insert into 4 child cluster

                    // Split
                    set_mb_black(get_parent(current));
                    set_mb_black(uncle);

                    current = get_grandparent(current);
                    set_mb_red(current);
                }
                else
                {
                    // Insert into 3 child cluster

                    if (current == get_right(get_parent(current)))
                    {
                        current = get_parent(current);
                        rbt_rotate_left(arena, current);
                    }

                    set_mb_black(get_parent(current));
                    set_mb_red(get_grandparent(current));
                    rbt_rotate_right(arena, get_grandparent(current));
                }
            }
            else
            {
                uncle = get_left(get_grandparent(current));

                if (is_red(uncle))
                {
                    // Insert into 4 child cluster

                    // Split
                    set_mb_black(get_parent(current));
                    set_mb_black(uncle);

                    current = get_grandparent(current);
                    set_mb_red(current);
                }
                else
                {
                    // Insert into 3 child cluster

                    if (current == get_left(get_parent(current)))
                    {
                        current = get_parent(current);
                        rbt_rotate_right(arena, current);
                    }

                    set_mb_black(get_parent(current));
                    set_mb_red(get_grandparent(current));

                    rbt_rotate_left(arena, get_grandparent(current));
                }
            }
        } while (is_red(get_parent(current)));
    }

    void rbt_insert(MemoryArena *arena, MemoryBlock *n)
    {
        if (arena->free_root == nullptr)
        {
            arena->free_root = n;
        }
        else
        {
            MemoryBlock *current = arena->free_root;
            MemoryBlock *parent  = nullptr;

            while (current != nullptr)
            {
                if (get_mb_size(current) == get_mb_size(n))
                {
                    add_center(current, n);
                    return;
                }

                parent  = current;
                current = get_mb_size(n) < get_mb_size(current) ? get_left(current) : get_right(current);
            }

            current = n;
            assign_left(current, nullptr);
            assign_right(current, nullptr);
            assign_parent(current, parent);
            set_mb_red(current);

            ASSERT(get_mb_size(current) != get_mb_size(parent), "Something went wrong internally. If the current size matches the size of the parent block, then we need to insert into the bucket!");
            if (get_mb_size(current) < get_mb_size(parent))
            {
                assign_left(parent, current);
            }
            else
            {
                assign_right(parent, current);
            }

            // Insertion into a red-black tree:
            //  0-children root cluster (parent node is BLACK) becomes 2-children root cluster (new root node)
            //    paint root node BLACK, and done
            //  2-children cluster (parent node is BLACK) becomes 3-children cluster
            //    done
            //  3-children cluster (parent node is BLACK) becomes 4-children cluster
            //    done
            //  3-children cluster (parent node is RED) becomes 4-children cluster
            //    rotate, and done
            //  4-children cluster (parent node is RED) splits into 2-children cluster and 3-children cluster
            //    split, and insert grandparent node into parent cluster

            if (is_red(get_parent(current)))
            {
                rbt_insert_repair(arena, current);
            }
            // else
            // {
            //		insertion into 0-children root cluster (parent node is BLACK)
            // 		insertion into 2-children cluster (parent node is BLACK)
            // 		insertion into 3-children cluster (parent node is BLACK)
            // }
        }
        set_mb_black(arena->free_root);
    }

} // namespace Vultr
