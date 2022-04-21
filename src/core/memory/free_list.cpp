#include "free_list.h"
#include <types/types.h>
#include <platform/platform.h>

namespace Vultr
{
	struct FreeListAllocator : public Allocator
	{
		// TODO(Brandon): Add support for 32 bit alignment (8 bytes)
		u8 alignment                    = 16;
		FreeListMemoryBlock *free_root  = nullptr;
		FreeListMemoryBlock *block_head = nullptr;
		size_t used                     = 0;
		Platform::Mutex mutex;

		FreeListAllocator() : Allocator(AllocatorType::FreeList) {}
	};

	// Allocated memory does not have any additional fields.
	struct AllocatedMemory
	{
	};

	struct FreeMemory
	{
		FreeListMemoryBlock *parent = nullptr;
		FreeListMemoryBlock *left   = nullptr;
		FreeListMemoryBlock *right  = nullptr;
		FreeListMemoryBlock *center = nullptr;
	};

	/*
	 * Present at the beginning of every block of memory
	 * Size: (64-bit) Allocated: 24 bytes, Free: 48 bytes.
	 * */
	struct FreeListMemoryBlock
	{
		// Lowest 3 bits of `size` store:
		// - Whether the block has properly initialized: 0 = Uninitialized, 1 = Initialized.
		// - Whether it is allocated or not:  0 = Freed, 1 = Allocated.
		// - The color of the block (Only valid for free blocks of memory. Used for red-black tree): 0 = Black, 1 = Red
		size_t size = 0;

		// Pointer to previous and next memory blocks
		FreeListMemoryBlock *prev = nullptr;
		FreeListMemoryBlock *next = nullptr;

		union
		{
			AllocatedMemory allocated;
			FreeMemory free;
		};
	};

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
#define HEADER_SIZE (sizeof(FreeListMemoryBlock) - sizeof(FreeMemory))
#define ASSERT_MB_INITIALIZED(block) ASSERT(BIT_IS_HIGH(block->size, INITIALIZED_BIT), "Memory block has not been initialized! Please call `init_free_mb` first!")
#define ASSERT_MB_FREE(block) ASSERT(BIT_IS_LOW(block->size, ALLOCATION_BIT), "Memory block is not free!")
#define ASSERT_MB_ALLOCATED(block) ASSERT(BIT_IS_HIGH(block->size, COLOR_BIT), "Memory block has not been allocated!")

	// Bit hack magic to manipulate lowest 3 bits of our size.
	// This is possible because alignment is 8 bytes minimum so the lowest 3 bits (1, 2, 4) will always be rounded to 0.
	// These can then be used to hold our initialization flag, allocation flag, and color bit
	static size_t get_mb_size(FreeListMemoryBlock *block)
	{
		ASSERT(block != nullptr, "Cannot get size of nullptr block.");
		return block->size & ~LOWEST_3_BITS;
	}
	static void *get_mb_memory(FreeListMemoryBlock *block)
	{
		ASSERT_MB_ALLOCATED(block);
		return block + HEADER_SIZE;
	}
	static size_t align(size_t size, u32 alignment)
	{
		// Without this, upon freeing a memory block of less than 16 bytes would cause overwriting of data from the next memory block.
		if (size < sizeof(FreeMemory))
		{
			return sizeof(FreeMemory);
		}

		auto remainder = size % alignment;

		if (remainder == 0)
			return size;

		return size + alignment - remainder;
	}
	static void set_mb_allocated(FreeListMemoryBlock *block) { block->size |= 1UL << ALLOCATION_BIT; }
	static bool mb_is_free(FreeListMemoryBlock *block)
	{
		if (block == nullptr)
			return false;
		return BIT_IS_LOW(block->size, ALLOCATION_BIT);
	}
	static void set_mb_free(FreeListMemoryBlock *block) { block->size &= ~(1UL << ALLOCATION_BIT); }
	static void set_mb_color(FreeListMemoryBlock *block, u8 color) { block->size = (block->size & ~(1UL << COLOR_BIT)) | (color << COLOR_BIT); }
	static void set_mb_black(FreeListMemoryBlock *block)
	{
		if (block == nullptr)
			return;
		block->size &= ~(1UL << COLOR_BIT);
	}
	static void set_mb_red(FreeListMemoryBlock *block)
	{
		ASSERT(block != nullptr, "Cannot set color of memory block nullptr");
		block->size |= 1UL << COLOR_BIT;
	}
	static bool is_red(FreeListMemoryBlock *block)
	{
		if (block == nullptr)
			return false;
		return BIT_IS_HIGH(block->size, COLOR_BIT);
	}
	static bool is_black(FreeListMemoryBlock *block) { return !is_red(block); }
	static void flip_color(FreeListMemoryBlock *block)
	{
		if (block == nullptr)
			return;
		block->size ^= (1UL << COLOR_BIT);
	}
	static void color_flip(FreeListMemoryBlock *block)
	{
		ASSERT_MB_FREE(block);
		flip_color(block);
		flip_color(block->free.left);
		flip_color(block->free.right);
	}

	static FreeListMemoryBlock *get_parent(FreeListMemoryBlock *block)
	{
		ASSERT(block != nullptr, "Cannot get parent from NULL block!");
		return block->free.parent;
	}
	static FreeListMemoryBlock *get_grandparent(FreeListMemoryBlock *block)
	{
		ASSERT(block != nullptr, "Cannot get grandparent from NULL block!");
		auto *parent = get_parent(block);
		ASSERT(parent != nullptr, "Block does not have a parent and thus doesn't have a grandparent!");
		return get_parent(parent);
	}
	static FreeListMemoryBlock *get_left(FreeListMemoryBlock *block)
	{
		ASSERT(block != nullptr, "Cannot get left from NULL block!");
		return block->free.left;
	}
	static FreeListMemoryBlock *get_right(FreeListMemoryBlock *block)
	{
		ASSERT(block != nullptr, "Cannot get right from NULL block!");
		return block->free.right;
	}

	static bool is_left_child(FreeListMemoryBlock *block)
	{
		ASSERT(block != nullptr, "Cannot determine if left child of NULL block!");
		auto *parent = get_parent(block);
		ASSERT(parent != nullptr, "Cannot determine if left child of NULL parent block!");
		return block == get_left(parent);
	}
	static bool is_right_child(FreeListMemoryBlock *block)
	{
		// NOTE(Brandon): This cannot just be !is_left_child because this function is also checking to make sure that the parent doesn't have a null child in the first place.
		ASSERT(block != nullptr, "Cannot determine if right child of NULL block!");
		auto *parent = get_parent(block);
		ASSERT(parent != nullptr, "Cannot determine if right child of NULL parent block!");
		return block == get_right(parent);
	}
	static FreeListMemoryBlock *get_sibling(FreeListMemoryBlock *block)
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
	static bool has_red_child(FreeListMemoryBlock *block)
	{
		ASSERT(block != nullptr, "Cannot determine if NULL block has a red child!");
		return is_red(get_left(block)) || is_red(get_right(block));
	}
	static FreeListMemoryBlock *get_center(FreeListMemoryBlock *block)
	{
		ASSERT(block != nullptr, "Cannot get center from NULL block!");
		return block->free.center;
	}
	static void assign_parent(FreeListMemoryBlock *dest, FreeListMemoryBlock *src)
	{
		ASSERT(dest != nullptr, "Cannot assign to NULL destination!");
		dest->free.parent = src;
	}
	static void assign_right(FreeListMemoryBlock *dest, FreeListMemoryBlock *src)
	{
		ASSERT(dest != nullptr, "Cannot assign to NULL destination!");
		dest->free.right = src;
		if (src != nullptr)
		{
			assign_parent(src, dest);
		}
	}
	static void assign_left(FreeListMemoryBlock *dest, FreeListMemoryBlock *src)
	{
		ASSERT(dest != nullptr, "Cannot assign to NULL destination!");
		dest->free.left = src;
		if (src != nullptr)
		{
			assign_parent(src, dest);
		}
	}
	static void assign_center(FreeListMemoryBlock *dest, FreeListMemoryBlock *src)
	{
		ASSERT(dest != nullptr, "Cannot assign to NULL destination!");
		dest->free.center = src;
		if (src != nullptr)
		{
			assign_parent(src, get_parent(dest));
		}
	}
	static void add_center(FreeListMemoryBlock *dest, FreeListMemoryBlock *src)
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

	static void rbt_print_binary_tree(const char *prefix, FreeListMemoryBlock *b, bool is_left)
	{
		if (b == nullptr)
			return;
		printf("%s", prefix);

		printf("%s", (is_left ? "├──" : "└──"));

		if (is_red(b))
			printf("\033[1;31m");

		printf("%lu\033[0m\n", get_mb_size(b));

		char new_prefix[strlen(prefix) + 5];
		strcpy(new_prefix, prefix);
		strcat(new_prefix, (is_left ? "│   " : "    "));

		rbt_print_binary_tree(new_prefix, get_left(b), true);
		rbt_print_binary_tree(new_prefix, get_right(b), false);
	}

	static u32 get_black_height(FreeListMemoryBlock *n)
	{
		u32 height = 0;
		while (n != nullptr)
		{
			if (is_black(n))
				height++;
			n = get_parent(n);
		}
		return height;
	}

	static FreeListMemoryBlock *node_first(FreeListAllocator *allocator)
	{
		auto *n = allocator->free_root;
		if (!n)
			return nullptr;

		while (get_left(n))
		{
			n = get_left(n);
		}
		return n;
	}

	static FreeListMemoryBlock *node_next(FreeListMemoryBlock *n)
	{
		if (get_right(n))
		{
			n = get_right(n);
			while (get_left(n))
			{
				n = get_left(n);
			}
			return n;
		}

		auto *parent = get_parent(n);
		while (parent && is_right_child(n))
		{
			n      = parent;
			parent = get_parent(n);
		}
		return parent;
	}
	static u32 count_black_nodes(FreeListMemoryBlock *n)
	{
		u32 black_nodes = 0;
		while (n)
		{
			if (is_black(n))
				black_nodes++;
			n = get_parent(n);
		}
		return black_nodes;
	}

	static bool test_rbt(FreeListAllocator *allocator)
	{
		auto *root   = allocator->free_root;
		bool success = true;

		if (is_red(root))
		{
			success = false;
			printf("The root is not black!\n");
		}

		auto *n         = node_first(allocator);
		u32 black_nodes = 0;

		{
			FreeListMemoryBlock *first_black_leaf = nullptr;
			FreeListMemoryBlock *parent           = nullptr;
			auto *i                               = n;
			while (i)
			{
				if (!get_left(i) && !get_right(i))
				{
					black_nodes      = count_black_nodes(i);
					first_black_leaf = i;
					break;
				}
				i = node_next(i);
			}
		}

		while (n)
		{
			auto *l = get_left(n);
			auto *r = get_right(n);
			if (l && get_mb_size(l) >= get_mb_size(n))
			{
				success = false;
				printf("Has left that is not less than memory block!");
			}
			if (r && get_mb_size(r) <= get_mb_size(n))
			{
				success = false;
				printf("Has right that is not greater than the memory block!");
			}
			if (!l)
			{
				if (r)
				{
					u32 bn = count_black_nodes(n);
					if (bn != black_nodes)
					{
						success = false;
						printf("Has a nullptr left child with a different black height\n");
					}
				}
			}
			else if (!r)
			{
				u32 bn = count_black_nodes(n);
				if (bn != black_nodes)
				{
					success = false;
					printf("Has a nullptr right childw ith a different black height\n");
				}
			}

			if (l && !r && is_black(l))
			{
				success = false;
				printf("Has one left child and it isn't red\n");
			}
			if (!l && r && is_black(r))
			{
				success = false;
				printf("Has one right child and it isn't red\n");
			}
			if (is_red(n))
			{
				if (is_red(l))
				{
					success = false;
					printf("Has a red left child and is red\n");
				}
				if (is_red(r))
				{
					success = false;
					printf("Has a red right child and is red\n");
				}
				auto *parent = get_parent(n);
				if (is_red(parent))
				{
					success = false;
					printf("Has a red parent and is red\n");
				}
			}

			if (!l && !r)
			{
				u32 bn = count_black_nodes(n);
				if (black_nodes != bn)
				{
					success = false;
					printf("Has a different black height\n");
				}
			}

			n = node_next(n);
		}
		if (!success)
		{
			rbt_print_binary_tree("", allocator->free_root, false);
		}
		return success;
	}

	static void rbt_insert(FreeListAllocator *allocator, FreeListMemoryBlock *n);
	static void insert_free_mb(FreeListAllocator *allocator, FreeListMemoryBlock *block)
	{
		ASSERT_MB_INITIALIZED(block);
		ASSERT_MB_FREE(block);
		rbt_insert(allocator, block);
		ASSERT(test_rbt(allocator), "Something went wrong with the RBT tree!");
		ASSERT(allocator->free_root != nullptr, "Something went wrong inserting memory block!");
		set_mb_black(allocator->free_root);
		assign_parent(allocator->free_root, nullptr);
	}
	static void rbt_delete(FreeListAllocator *allocator, FreeListMemoryBlock *n);
	static void remove_free_mb(FreeListAllocator *allocator, FreeListMemoryBlock *block)
	{
		ASSERT_MB_INITIALIZED(block);
		ASSERT_MB_FREE(block);
		rbt_delete(allocator, block);
		ASSERT(test_rbt(allocator), "Something went wrong with the RBT tree!");
		if (allocator->free_root != nullptr)
		{
			set_mb_black(allocator->free_root);
		}
	}
	static void rbt_update(FreeListAllocator *allocator, FreeListMemoryBlock *n, size_t new_size);

	static FreeListMemoryBlock *mb_best_match(FreeListMemoryBlock *h, size_t size)
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

			// If the smaller block is too small, then just return the head.
			if (get_mb_size(l) < size)
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

	static void init_free_mb(FreeListMemoryBlock *block, size_t size, FreeListMemoryBlock *prev, FreeListMemoryBlock *next)
	{
		block->size        = (~(1UL << ALLOCATION_BIT) & size) | (1UL << INITIALIZED_BIT);
		block->next        = next;
		block->prev        = prev;
		block->free.parent = nullptr;
		block->free.center = nullptr;
		block->free.left   = nullptr;
		block->free.right  = nullptr;
	}

	FreeListAllocator *init_free_list_allocator(MemoryArena *arena, size_t size, u8 alignment)
	{
		// Designate a region within the memory arena for our allocator.
		auto *allocator = static_cast<FreeListAllocator *>(mem_arena_designate(arena, AllocatorType::FreeList, size + sizeof(FreeListAllocator)));

		// If we were unable to allocate the required size, then there is nothing to do.
		if (allocator == nullptr)
			return nullptr;

		// Set the type of allocator.
		allocator->type = AllocatorType::FreeList;

		// Set up the alignment.
		allocator->alignment = alignment;

		// The head block will come after the memory allocator.
		allocator->block_head = reinterpret_cast<FreeListMemoryBlock *>(reinterpret_cast<byte *>(allocator) + sizeof(FreeListAllocator));
		allocator->free_root  = nullptr;

		// Memory block head.
		auto *h = allocator->block_head;

		// Subtract the size of the memory header because this will exist at all times
		init_free_mb(h, size - HEADER_SIZE, nullptr, nullptr);
		insert_free_mb(allocator, h);

		return allocator;
	}

	static FreeListMemoryBlock *split_mb(FreeListAllocator *allocator, FreeListMemoryBlock *b, size_t new_size)
	{
		size_t old_size = get_mb_size(b);

		// If the new size is exactly the same size as our memory block, there is no reason to split.
		if (new_size == old_size)
		{
			rbt_delete(allocator, b);
			ASSERT(test_rbt(allocator), "Something went wrong with the RBT tree!");
			return b;
		}

		// If this block is not big enough to be split into another smaller memory block, don't bother...
		// TODO(Brandon): Add test case for this.
		if (old_size - new_size < sizeof(FreeListMemoryBlock))
		{
			rbt_delete(allocator, b);
			ASSERT(test_rbt(allocator), "Something went wrong with the RBT tree!");
			return b;
		}

		size_t updated_size = old_size - new_size - HEADER_SIZE;
		rbt_update(allocator, b, updated_size);

		auto *new_block = reinterpret_cast<FreeListMemoryBlock *>(reinterpret_cast<byte *>(b) + updated_size + HEADER_SIZE);
		init_free_mb(new_block, new_size, b, b->next);

		if (new_block->next != nullptr)
		{
			new_block->next->prev = new_block;
		}

		b->next = new_block;
		return new_block;
	}

	static void coalesce_mbs(FreeListAllocator *allocator, FreeListMemoryBlock *b)
	{
		auto *prev     = b->prev;
		auto prev_size = b->prev ? get_mb_size(prev) : 0;
		auto *next     = b->next;
		auto next_size = b->next ? get_mb_size(next) : 0;
		auto b_size    = get_mb_size(b);

		if (mb_is_free(prev) && mb_is_free(next))
		{
			size_t new_size = prev_size + (b_size + HEADER_SIZE) + (next_size + HEADER_SIZE);
			remove_free_mb(allocator, prev);
			remove_free_mb(allocator, next);

			init_free_mb(prev, new_size, prev->prev, next->next);
			insert_free_mb(allocator, prev);

			if (next->next != nullptr)
			{
				next->next->prev = prev;
			}
		}
		else if (mb_is_free(prev))
		{
			size_t new_size = prev_size + (b_size + HEADER_SIZE);
			remove_free_mb(allocator, prev);

			init_free_mb(prev, new_size, prev->prev, next);
			insert_free_mb(allocator, prev);

			if (next != nullptr)
			{
				next->prev = prev;
			}
		}
		else if (mb_is_free(next))
		{
			size_t new_size = b_size + (next_size + HEADER_SIZE);
			remove_free_mb(allocator, next);

			init_free_mb(b, new_size, prev, next->next);
			insert_free_mb(allocator, b);

			if (prev != nullptr)
			{
				prev->next = b;
			}
		}
		else
		{
			insert_free_mb(allocator, b);
		}
	}
	static void *free_list_alloc_no_lock(FreeListAllocator *allocator, size_t size)
	{
		size = align(size, allocator->alignment);
		printf("Request to allocate %zu bytes\n", size);
		// Find a memory block of suitable size.
		auto *best_match = mb_best_match(allocator->free_root, size);
		PRODUCTION_ASSERT(best_match != nullptr, "Not enough memory to allocate!");
		ASSERT(get_mb_size(best_match) >= size, "");

		// If need be, split the memory block into the size that we need.
		auto *new_block = split_mb(allocator, best_match, size);

		// Set our memory block to allocated.
		set_mb_allocated(new_block);
		allocator->used += size;
		void *data = reinterpret_cast<byte *>(new_block) + HEADER_SIZE;
		printf("Allocated %zu bytes at %p\n", size, data);
		free_list_print(allocator);
		ASSERT(test_rbt(allocator), "Something went wrong with the RBT tree!");
		return data;
	}

	// TODO(Brandon): Add lots of error messages to make sure this isn't misused.
	void *free_list_alloc(FreeListAllocator *allocator, size_t size)
	{
		Platform::Lock l(allocator->mutex);
		return free_list_alloc_no_lock(allocator, size);
	}

	static FreeListMemoryBlock *get_block_from_allocated_data(void *data) { return reinterpret_cast<FreeListMemoryBlock *>(reinterpret_cast<byte *>(data) - HEADER_SIZE); }
	static void free_list_free_no_lock(FreeListAllocator *allocator, void *data)
	{
		auto *block_to_free = get_block_from_allocated_data(data);
		size_t size         = get_mb_size(block_to_free);
		printf("Freeing %zu bytes at %p\n", size, data);
		auto *prev = block_to_free->prev;
		auto *next = block_to_free->next;
		init_free_mb(block_to_free, size, prev, next);
		coalesce_mbs(allocator, block_to_free);
		allocator->used -= size;
		free_list_print(allocator);
		ASSERT(test_rbt(allocator), "Something went wrong with the RBT tree!");
	}
	void free_list_free(FreeListAllocator *allocator, void *data)
	{
		Platform::Lock l(allocator->mutex);
		PRODUCTION_ASSERT(data != nullptr, "Cannot free nullptr memory!");
		free_list_free_no_lock(allocator, data);
	}

	void *free_list_realloc(FreeListAllocator *allocator, void *data, size_t size)
	{
		Platform::Lock l(allocator->mutex);
		auto *block         = get_block_from_allocated_data(data);
		size_t current_size = get_mb_size(block);

		auto *next          = block->next;
		auto next_is_free   = next != nullptr && mb_is_free(next);
		auto next_size      = next ? get_mb_size(next) : 0;

		if (next_is_free && next_size + current_size + HEADER_SIZE >= size)
		{
			return data;
		}
		else
		{
			void *new_data = free_list_alloc_no_lock(allocator, size);
			memcpy(new_data, data, current_size);
			free_list_free_no_lock(allocator, data);
			return new_data;
		}
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

	static bool bst_insert(FreeListAllocator *allocator, FreeListMemoryBlock *n)
	{
		auto *h = allocator->free_root;
		if (h == nullptr)
		{
			allocator->free_root = n;
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

	static void rbt_right_rotate(FreeListAllocator *allocator, FreeListMemoryBlock *n)
	{
		auto *l = get_left(n);
		assign_left(n, get_right(l));

		assign_parent(l, get_parent(n));

		u8 old_n_color = is_red(n);
		set_mb_color(n, is_red(l));
		set_mb_color(l, old_n_color);

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
		// If it is, then we need to fix the root node pointed to in memory allocator.
		else
		{
			allocator->free_root = l;
		}

		assign_right(l, n);
	}

	static void rbt_left_rotate(FreeListAllocator *allocator, FreeListMemoryBlock *n)
	{
		auto *r = get_right(n);
		assign_right(n, get_left(r));

		assign_parent(r, get_parent(n));

		u8 old_n_color = is_red(n);
		set_mb_color(n, is_red(r));
		set_mb_color(r, old_n_color);

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
		// If it is, then we need to fix the root node pointed to in memory allocator.
		else
		{
			allocator->free_root = r;
		}
		assign_left(r, n);
	}

	void rbt_insert(FreeListAllocator *allocator, FreeListMemoryBlock *n)
	{
		auto can_skip = bst_insert(allocator, n);
		if (can_skip)
		{
			set_mb_black(allocator->free_root);
			return;
		}

		FreeListMemoryBlock *parent      = nullptr;
		FreeListMemoryBlock *grandparent = nullptr;

		// Traverse until either:
		// - We reach the root node.
		// - The current node is black.
		// - The current node's parent is black.

		while (n != allocator->free_root && is_red(n) && is_red(get_parent(n)))
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
						rbt_left_rotate(allocator, parent);
						n      = parent;
						parent = get_parent(n);
					}

					// If n is a left child of its parent...
					rbt_right_rotate(allocator, grandparent);
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
						rbt_right_rotate(allocator, parent);
						n      = parent;
						parent = get_parent(n);
					}

					// If n is a right child of its parent...
					rbt_left_rotate(allocator, grandparent);
					n = parent;
				}
			}
		}

		set_mb_black(allocator->free_root);
	}

	static void mb_replace_child(FreeListAllocator *allocator, FreeListMemoryBlock *n, FreeListMemoryBlock *child)
	{
		auto *parent = get_parent(n);
		if (parent != nullptr)
		{
			if (is_left_child(n))
			{
				assign_left(parent, child);
			}
			else
			{
				assign_right(parent, child);
			}
		}
		else
		{
			allocator->free_root = child;
			assign_parent(child, nullptr);
		}
		set_mb_color(child, is_red(n) ? RED : BLACK);
	}

	static void rbt_fix_double_black(FreeListAllocator *allocator, FreeListMemoryBlock *parent, FreeListMemoryBlock *n)
	{
		FreeListMemoryBlock *sibling = nullptr;
		if (get_right(parent) != n)
		{
			sibling = get_right(parent);
			if (is_red(sibling))
			{
				rbt_left_rotate(allocator, parent);
				sibling = get_right(parent);
			}

			if (is_red(get_right(sibling)))
			{
				set_mb_black(get_right(sibling));
				rbt_left_rotate(allocator, parent);
			}
			else if (is_red(get_left(sibling)))
			{
				rbt_right_rotate(allocator, sibling);
				rbt_left_rotate(allocator, parent);
				set_mb_black(sibling);
			}
			else
			{
				set_mb_red(sibling);
				if (get_parent(parent) != nullptr && is_black(parent))
				{
					rbt_fix_double_black(allocator, get_parent(parent), parent);
				}
				else
				{
					set_mb_black(parent);
				}
			}
		}
		else
		{
			sibling = get_left(parent);
			if (is_red(sibling))
			{
				rbt_right_rotate(allocator, parent);
				sibling = get_left(parent);
			}
			if (is_red(get_left(sibling)))
			{
				set_mb_black(get_left(sibling));
				rbt_right_rotate(allocator, parent);
			}
			else if (is_red(get_right(sibling)))
			{
				rbt_left_rotate(allocator, sibling);
				rbt_right_rotate(allocator, parent);
				set_mb_black(sibling);
			}
			else
			{
				set_mb_red(sibling);
				if (get_parent(parent) && is_black(parent))
				{
					rbt_fix_double_black(allocator, get_parent(parent), parent);
				}
				else
				{
					set_mb_black(parent);
				}
			}
		}
	}

	void rbt_delete(FreeListAllocator *allocator, FreeListMemoryBlock *n)
	{
		auto *parent = get_parent(n);
		auto *l      = get_left(n);
		auto *r      = get_right(n);

		auto *c      = get_center(n);
		if (c != nullptr)
		{
			assign_center(n, nullptr);
			assign_left(c, l);
			assign_right(c, r);
			if (parent != nullptr)
			{
				if (is_left_child(n))
				{
					assign_left(parent, n);
				}
				else // if (is_right_child(n))
				{
					assign_right(parent, n);
				}
				set_mb_color(c, is_red(n) ? RED : BLACK);
			}
			else
			{
				assign_parent(c, nullptr);
				allocator->free_root = c;
				set_mb_black(c);
			}
			return;
		}

		if (l == nullptr)
		{
			if (r != nullptr)
			{
				mb_replace_child(allocator, n, r);
			}
			else
			{
				if (parent != nullptr)
				{
					if (is_left_child(n))
					{
						assign_left(parent, nullptr);
					}
					else
					{
						assign_right(parent, nullptr);
					}

					if (is_black(n))
						rbt_fix_double_black(allocator, parent, nullptr);
				}
				else
				{
					allocator->free_root = nullptr;
				}
			}
		}
		else if (r == nullptr)
		{
			mb_replace_child(allocator, n, l);
		}
		else
		{
			auto *successor = r;
			if (get_left(successor) == nullptr)
			{
				bool black = is_black(successor);
				mb_replace_child(allocator, n, successor);

				assign_left(successor, l);
				if (get_right(successor) != nullptr)
				{
					set_mb_black(get_right(successor));
				}
				else
				{
					if (black)
						rbt_fix_double_black(allocator, successor, nullptr);
				}
			}
			else
			{
				while (get_left(successor) != nullptr)
					successor = get_left(successor);

				bool black    = is_black(successor);
				auto *sr      = get_right(successor);
				auto *sparent = get_parent(successor);
				assign_left(sparent, sr);

				if (sr)
				{
					set_mb_black(sr);
					black = false;
				}

				mb_replace_child(allocator, n, successor);
				assign_left(successor, l);
				assign_right(successor, r);
				if (black)
					rbt_fix_double_black(allocator, sparent, nullptr);
			}
		}
		set_mb_black(allocator->free_root);
	}

	void rbt_update(FreeListAllocator *allocator, FreeListMemoryBlock *n, size_t new_size)
	{
		rbt_delete(allocator, n);
		ASSERT(test_rbt(allocator), "Something went wrong with the RBT tree!");

		init_free_mb(n, new_size, n->prev, n->next);

		rbt_insert(allocator, n);
		ASSERT(test_rbt(allocator), "Something went wrong with the RBT tree!");
	}

	void free_list_print(FreeListAllocator *allocator) { rbt_print_binary_tree("", allocator->free_root, false); }
} // namespace Vultr
