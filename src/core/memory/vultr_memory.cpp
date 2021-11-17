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
#define ASSERT_MB_FREE(block) ASSERT(BIT_IS_LOW(block->size, ALLOCATION_BIT), "Memory block is not free!")
#define ASSERT_MB_ALLOCATED(block) ASSERT(BIT_IS_HIGH(block->size, COLOR_BIT), "Memory block has not been allocated!")

    // Bit hack magic to manipulate lowest 3 bits of our size.
    // This is possible because alignment is 8 bytes minimum so the lowest 3 bits (1, 2, 4) will always be rounded to 0.
    // These can then be used to hold our initialization flag, allocation flag, and color bit
    static size_t get_mb_size(MemoryBlock *block) { return block->size & ~LOWEST_3_BITS; }
    static void *get_mb_memory(MemoryBlock *block)
    {
        ASSERT_MB_ALLOCATED(block);
        return block + HEADER_SIZE;
    }
    static size_t align(size_t size, u32 alignment)
    {
        auto remainder = size % alignment;

        if (remainder == 0)
            return size;

        return size + alignment - remainder;
    }
    static void set_mb_allocated(MemoryBlock *block) { block->size |= 1UL << ALLOCATION_BIT; }
    static bool mb_is_free(MemoryBlock *block)
    {
        if (block == nullptr)
            return false;
        return BIT_IS_LOW(block->size, ALLOCATION_BIT);
    }
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
        ASSERT(block != nullptr, "Cannot get parent from NULL block!");
        return block->free.parent;
    }
    static MemoryBlock *get_grandparent(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot get grandparent from NULL block!");
        auto *parent = get_parent(block);
        ASSERT(parent != nullptr, "Block does not have a parent and thus doesn't have a grandparent!");
        return get_parent(parent);
    }
    static MemoryBlock *get_left(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot get left from NULL block!");
        return block->free.left;
    }
    static MemoryBlock *get_right(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot get right from NULL block!");
        return block->free.right;
    }

    static bool is_left_child(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot determine if left child of NULL block!");
        auto *parent = get_parent(block);
        ASSERT(parent != nullptr, "Cannot determine if left child of NULL parent block!");
        return block == get_left(parent);
    }
    static bool is_right_child(MemoryBlock *block)
    {
        // NOTE(Brandon): This cannot just be !is_left_child because this function is also checking to make sure that the parent doesn't have a null child in the first place.
        ASSERT(block != nullptr, "Cannot determine if right child of NULL block!");
        auto *parent = get_parent(block);
        ASSERT(parent != nullptr, "Cannot determine if right child of NULL parent block!");
        return block == get_right(parent);
    }
    static MemoryBlock *get_sibling(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot get grandparent from NULL block!");
        auto *parent = get_parent(block);

        if (parent == nullptr)
            return nullptr;

        if (is_left_child(block))
        {
            return get_right(parent);
        }
        else
        {
            return get_left(parent);
        }
    }
    static bool has_red_child(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot determine if NULL block has a red child!");
        return is_red(get_left(block)) || is_red(get_right(block));
    }
    static MemoryBlock *get_center(MemoryBlock *block)
    {
        ASSERT(block != nullptr, "Cannot get center from NULL block!");
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

    static void rbt_insert(MemoryArena *arena, MemoryBlock *n);
    void insert_free_mb(MemoryArena *arena, MemoryBlock *block)
    {
        ASSERT_MB_INITIALIZED(block);
        ASSERT_MB_FREE(block);
        rbt_insert(arena, block);
        ASSERT(arena->free_root != nullptr, "Something went wrong inserting memory block!");
        set_mb_black(arena->free_root);
        assign_parent(arena->free_root, nullptr);
    }
    static void rbt_delete(MemoryArena *arena, MemoryBlock *n);
    void remove_free_mb(MemoryArena *arena, MemoryBlock *block)
    {
        ASSERT_MB_INITIALIZED(block);
        ASSERT_MB_FREE(block);
        rbt_delete(arena, block);
        if (arena->free_root != nullptr)
        {
            set_mb_black(arena->free_root);
        }
    }

    MemoryBlock *mb_best_match(MemoryBlock *h, size_t size)
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

    void init_free_mb(MemoryBlock *block, size_t size, MemoryBlock *prev, MemoryBlock *next)
    {
        block->size        = (~(1UL << ALLOCATION_BIT) & size) | (1UL << INITIALIZED_BIT);
        block->next        = next;
        block->prev        = prev;
        block->free.parent = nullptr;
        block->free.center = nullptr;
        block->free.left   = nullptr;
        block->free.right  = nullptr;
    }

    MemoryArena *init_mem_arena(size_t size, u8 alignment)
    {
        ASSERT(size > sizeof(MemoryBlock), "Why are you initializing a memory arena that is literally smaller than 48 bytes...");

        // Virtual alloc some memory.
        auto *memory_block = Platform::virtual_alloc(nullptr, sizeof(MemoryArena) + size);

        // If it returned nullptr, then we can assume that the allocation failed.
        if (memory_block == nullptr)
        {
            return nullptr;
        }

        // The memory arena will be at the start of this platform memory block.
        auto *arena      = reinterpret_cast<MemoryArena *>(Platform::get_memory(memory_block));
        arena->alignment = alignment;

        // And in that arena we will have a pointer to the head of the platform memory block.
        arena->memory = memory_block;

        // The head block will come after the memory arena.
        arena->block_head = reinterpret_cast<MemoryBlock *>(reinterpret_cast<char *>(arena) + sizeof(MemoryArena));

        // Memory block head.
        auto *h = arena->block_head;

        // Subtract the size of the memory header because this will exist at all times
        init_free_mb(h, size - HEADER_SIZE, nullptr, nullptr);
        insert_free_mb(arena, h);

        return arena;
    }

    static MemoryBlock *split_mb(MemoryBlock *b, size_t new_size)
    {
        // If the new size is exactly the same size as our memory block, there is no reason to split.
        if (new_size == get_mb_size(b))
        {
            return nullptr;
        }
        u32 lowest_bits = b->size & LOWEST_3_BITS;
        u32 old_size    = get_mb_size(b);

        // If this block is not big enough to be split into another smaller memory block, don't bother...
        // TODO(Brandon): Add test case for this.
        if (old_size - new_size < sizeof(MemoryBlock))
        {
            return nullptr;
        }

        b->size = new_size | lowest_bits;

        MemoryBlock *new_block = reinterpret_cast<MemoryBlock *>(reinterpret_cast<char *>(b) + new_size + HEADER_SIZE);
        init_free_mb(new_block, old_size - new_size - HEADER_SIZE, b, b->next);

        b->next = new_block;
        return new_block;
    }

    static void coalesce_mbs(MemoryArena *arena, MemoryBlock *b)
    {
        auto *prev     = b->prev;
        auto prev_size = b->prev ? get_mb_size(prev) : 0;
        auto *next     = b->next;
        auto next_size = b->next ? get_mb_size(next) : 0;
        auto b_size    = get_mb_size(b);

        if (mb_is_free(prev) && mb_is_free(next))
        {
            size_t new_size = prev_size + (b_size + HEADER_SIZE) + (next_size + HEADER_SIZE);
            remove_free_mb(arena, prev);
            remove_free_mb(arena, next);

            init_free_mb(prev, new_size, prev->prev, next->next);
            insert_free_mb(arena, prev);

            if (next->next != nullptr)
            {
                next->next->prev = prev;
            }
        }
        else if (mb_is_free(prev))
        {
            size_t new_size = prev_size + (b_size + HEADER_SIZE);
            remove_free_mb(arena, prev);

            init_free_mb(prev, new_size, prev->prev, next);
            insert_free_mb(arena, prev);

            if (next != nullptr)
            {
                next->prev = prev;
            }
        }
        else if (mb_is_free(next))
        {
            size_t new_size = b_size + (next_size + HEADER_SIZE);
            remove_free_mb(arena, next);

            init_free_mb(b, new_size, prev, next->next);
            insert_free_mb(arena, b);

            if (prev != nullptr)
            {
                prev->next = b;
            }
        }
        else
        {
            insert_free_mb(arena, b);
        }
    }

    // TODO(Brandon): Add lots of error messages to make sure this isn't misused.
    void *mem_arena_alloc(MemoryArena *arena, size_t size)
    {
        size = align(size, arena->alignment);
        // Find a memory block of suitable size.
        auto *best_match = mb_best_match(arena->free_root, size);
        PRODUCTION_ASSERT(best_match != nullptr, "Not enough memory to allocate!");
        ASSERT(get_mb_size(best_match) >= size, "");

        // Delete this memory block from the red black tree.
        remove_free_mb(arena, best_match);

        // If need be, split the memory block into the size that we need.
        auto *new_block = split_mb(best_match, size);
        if (new_block != nullptr)
        {
            // Insert this new memory block as free into the memory arena.
            insert_free_mb(arena, new_block);
        }

        // Set our memory block to allocated.
        set_mb_allocated(best_match);
        arena->used += size;
        return reinterpret_cast<char *>(best_match) + HEADER_SIZE;
    }

    static MemoryBlock *get_block_from_allocated_data(void *data) { return reinterpret_cast<MemoryBlock *>(reinterpret_cast<char *>(data) - HEADER_SIZE); }

    void *mem_arena_realloc(MemoryArena *arena, void *data, size_t size)
    {
        auto *block         = get_block_from_allocated_data(data);
        size_t current_size = get_mb_size(block);

        auto *next        = block->next;
        auto next_is_free = next ? mb_is_free(next) : false;
        auto next_size    = next ? get_mb_size(next) : 0;

        if (next_is_free && next_size + current_size + HEADER_SIZE >= size)
        {
            return data;
        }
        else
        {
            void *new_data = mem_arena_alloc(arena, size);
            mem_copy(data, new_data, current_size);
            mem_arena_free(arena, data);
            return new_data;
        }
    }

    void mem_copy(void *src, void *dest, size_t len) {}

    void mem_arena_free(MemoryArena *arena, void *data)
    {
        auto *block_to_free = get_block_from_allocated_data(data);
        size_t size         = get_mb_size(block_to_free);
        auto *prev          = block_to_free->prev;
        auto *next          = block_to_free->next;
        init_free_mb(block_to_free, size, prev, next);
        coalesce_mbs(arena, block_to_free);
    }
    void destroy_mem_arena(MemoryArena *arena)
    {
        ASSERT(arena != nullptr && arena->memory != nullptr, "Invalid memory arena!");
        Platform::virtual_free(arena->memory);
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

    static bool bst_insert(MemoryArena *arena, MemoryBlock *n)
    {
        auto *h = arena->free_root;
        if (h == nullptr)
        {
            arena->free_root = n;
            set_mb_black(n);
            return true;
        }

        set_mb_red(n);

        while (h != nullptr)
        {
            size_t h_size = get_mb_size(h);
            size_t n_size = get_mb_size(n);
            if (n_size < h_size)
            {
                auto *l = get_left(h);
                if (l == nullptr)
                {
                    assign_left(h, n);
                    return false;
                }
                else
                {
                    h = l;
                }
            }
            else if (n_size > h_size)
            {
                auto *r = get_right(h);
                if (r == nullptr)
                {
                    assign_right(h, n);
                    return false;
                }
                else
                {
                    h = r;
                }
            }
            else if (n_size == h_size)
            {
                add_center(h, n);
                return true;
            }
        }

        return false;
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
        auto can_skip = bst_insert(arena, n);
        if (can_skip)
        {
            return;
        }

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

    static MemoryBlock *bst_get_successor(MemoryBlock *n)
    {
        // Find node that doesn't have a left child in the given tree.
        MemoryBlock *x = n;

        while (get_left(x) != nullptr)
        {
            x = get_left(x);
        }

        return x;
    }

    static MemoryBlock *bst_find_replacement(MemoryBlock *n)
    {
        ASSERT(get_center(n) == nullptr, "The replacement for this block should have been just it's center, however this method is being called meaning something went wrong.");

        // When the node has 2 children.
        if (get_left(n) != nullptr && get_right(n) != nullptr)
        {
            return bst_get_successor(get_right(n));
        }

        // When the node is a leaf, just return nullptr since there is no node to replace it.
        if (get_left(n) == nullptr && get_right(n) == nullptr)
        {
            return nullptr;
        }

        // When the node has a single child return that child.
        if (get_left(n) != nullptr)
        {
            return get_left(n);
        }
        else // if (get_right(n) != nullptr)
        {
            return get_right(n);
        }
    }

    static void fix_double_black(MemoryArena *arena, MemoryBlock *n)
    {
        while (1)
        {
            // Recurse until reach the root.
            if (n == arena->free_root)
                return;

            auto *sibling = get_sibling(n);
            auto *parent  = get_parent(n);

            // If there is no sibling, double black is pushed up the tree.
            if (sibling == nullptr)
            {
                n = parent;
                continue;
            }
            else
            {
                // If sibling is red...
                if (is_red(sibling))
                {
                    set_mb_red(parent);
                    set_mb_black(sibling);

                    // Left case
                    if (is_left_child(sibling))
                    {
                        rbt_right_rotate(arena, parent);
                    }
                    else
                    {
                        rbt_left_rotate(arena, parent);
                    }

                    // Fix double black again.
                    continue;
                }
                // If sibling is black...
                else
                {
                    // If there is at least one red child...
                    if (has_red_child(sibling))
                    {
                        // Left case...
                        if (is_red(get_left(sibling)))
                        {
                            // Left left
                            if (is_left_child(sibling))
                            {
                                set_mb_color(get_left(sibling), is_red(sibling));
                                set_mb_color(sibling, is_red(parent));
                                rbt_right_rotate(arena, parent);
                            }
                            // Right left
                            else
                            {
                                set_mb_color(get_left(sibling), is_red(parent));
                                rbt_right_rotate(arena, sibling);
                                rbt_left_rotate(arena, parent);
                            }
                        }
                        // Right case...
                        else
                        {
                            // Left right
                            if (is_left_child(sibling))
                            {
                                set_mb_color(get_right(sibling), is_red(parent));
                                rbt_left_rotate(arena, sibling);
                                rbt_right_rotate(arena, parent);
                            }
                            // Right right
                            else
                            {
                                set_mb_color(get_right(sibling), is_red(sibling));
                                set_mb_color(sibling, is_red(parent));
                                rbt_left_rotate(arena, parent);
                            }
                        }
                        set_mb_black(parent);
                    }
                    // If there are two black children...
                    else
                    {
                        set_mb_red(sibling);
                        if (is_black(parent))
                        {
                            n = parent;
                            continue;
                        }
                        else
                        {
                            set_mb_black(parent);
                        }
                    }
                }
            }
        }
    }

    static void swap_nodes(MemoryArena *arena, MemoryBlock *x, MemoryBlock *y)
    {
        ASSERT(x != nullptr && y != nullptr, "Cannot swap NULL nodes!");
        auto *x_parent = get_parent(x);
        auto x_color   = is_red(x);
        auto *x_left   = get_left(x);
        auto *x_right  = get_right(x);
        auto x_is_lc   = x_parent != nullptr ? is_left_child(x) : false;

        auto *y_parent = get_parent(y);
        auto y_color   = is_red(y);
        auto *y_left   = get_left(y);
        auto *y_right  = get_right(y);
        auto y_is_lc   = y_parent != nullptr ? is_left_child(y) : false;

        if (y_parent == x)
        {
            y_parent = y;
        }
        if (x_parent == y)
        {
            x_parent = x;
        }

        assign_parent(x, y_parent);
        set_mb_color(x, y_color);
        if (y_left == x)
        {
            assign_left(x, y);
        }
        else
        {
            assign_left(x, y_left);
        }
        if (y_right == x)
        {
            assign_right(x, y);
        }
        else
        {
            assign_right(x, y_right);
        }
        if (y_parent != nullptr)
        {
            if (y_is_lc)
            {
                assign_left(y_parent, x);
            }
            else
            {
                assign_right(y_parent, x);
            }
        }
        else
        {
            ASSERT(x_parent != nullptr, "Both cannot be roots...");
            arena->free_root = x;
        }

        assign_parent(y, x_parent);
        set_mb_color(y, x_color);
        if (x_left == y)
        {
            assign_left(y, x);
        }
        else
        {
            assign_left(y, x_left);
        }
        if (x_right == y)
        {
            assign_right(y, x);
        }
        else
        {
            assign_right(y, x_right);
        }
        if (x_parent != nullptr)
        {
            if (x_is_lc)
            {
                assign_left(x_parent, y);
            }
            else
            {
                assign_right(x_parent, y);
            }
        }
        else
        {
            ASSERT(y_parent != nullptr, "Both cannot be roots...");
            arena->free_root = y;
        }
    }

    void rbt_delete(MemoryArena *arena, MemoryBlock *n)
    {
        while (1)
        {
            ASSERT(n != nullptr, "Cannot delete null memory block!");

            auto *replacement = bst_find_replacement(n);

            bool both_black = is_black(replacement) && is_black(n);
            auto *parent    = get_parent(n);
            auto *root      = arena->free_root;

            // If replacement is nullptr then n is a leaf.
            if (replacement == nullptr)
            {
                // If the root is nullptr...
                if (n == root)
                {
                    // Then update the free root in the memory arena.
                    arena->free_root = nullptr;
                }
                else
                {
                    // If both are black, then it needs to be fixed.
                    // n is a leaf, thus we need to fix the double black at n.
                    if (both_black)
                    {
                        fix_double_black(arena, n);
                    }
                    // If either n or replacement are red...
                    else
                    {
                        auto *sibling = get_sibling(n);
                        // If the sibling exists, make it red.
                        if (sibling != nullptr)
                        {
                            set_mb_red(sibling);
                        }
                    }

                    // Delete n from the tree.
                    if (is_left_child(n))
                    {
                        assign_left(parent, nullptr);
                    }
                    else
                    {
                        assign_right(parent, nullptr);
                    }
                }
                return;
            }
            // If n has one child...
            else if (get_left(n) == nullptr || get_right(n) == nullptr)
            {
                if (n == root)
                {
                    assign_parent(replacement, nullptr);
                    set_mb_black(replacement);
                    assign_left(replacement, nullptr);
                    assign_right(replacement, nullptr);
                    arena->free_root = replacement;
                }
                else
                {
                    if (is_left_child(n))
                    {
                        assign_left(parent, replacement);
                    }
                    else
                    {
                        assign_right(parent, replacement);
                    }

                    assign_parent(replacement, parent);

                    // If both are black, then fix the double black at the replacement.
                    if (both_black)
                    {
                        fix_double_black(arena, replacement);
                    }
                    // If either are red, then set the replacement to black.
                    else
                    {
                        set_mb_black(replacement);
                    }
                }
                return;
            }
            // If n has two children...
            else
            {
                swap_nodes(arena, n, replacement);
            }
        }
    }
} // namespace Vultr
