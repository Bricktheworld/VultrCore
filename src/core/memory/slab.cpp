#include "slab.h"
#include <math/integer_division.h>
#include <utils/transfer.h>

namespace Vultr
{
	struct Slab
	{
		size_t count            = 0;
		size_t free_blocks_len  = 0;
		size_t block_size       = 0;
		atomic_u64 *free_blocks = nullptr;
		void *heap              = nullptr;
	};

	static size_t get_slab_size(SlabDeclaration *declaration)
	{
		size_t heap   = declaration->block_size * declaration->count;
		size_t header = sizeof(Slab) + sizeof(atomic_u64) * int_ceil_divide(declaration->count, 64);
		return heap + header;
	}

	static void *init_slab(Slab *slab, SlabDeclaration *declaration)
	{
		// TODO(Brandon): Don't do this.
		ASSERT(declaration->count % 64 == 0, "Slab allocator only supports multiples of 64");
		ASSERT((declaration->block_size & 0x7) == 0, "Slab allocator only supports alignments of 16 bits");
		slab->count           = declaration->count;
		slab->free_blocks_len = int_ceil_divide(slab->count, 64);
		slab->free_blocks     = reinterpret_cast<atomic_u64 *>(slab + 1);
		slab->heap            = reinterpret_cast<byte *>(slab->free_blocks) + sizeof(atomic_u64) * slab->free_blocks_len;
		slab->block_size      = declaration->block_size;
		for (size_t i = 0; i < slab->free_blocks_len; i++)
		{
			new (&slab->free_blocks[i]) atomic_u64(0);
		}
		return reinterpret_cast<byte *>(slab->heap) + slab->block_size * slab->count;
	}

	SlabAllocator *init_slab_allocator(MemoryArena *arena, SlabDeclaration *declarations, u32 region_count)
	{
		// TODO(Brandon): Sort the declarations.
		size_t header_size = sizeof(SlabAllocator) + (sizeof(Slab *) * region_count);
		size_t total_size  = header_size;
		for (u32 i = 0; i < region_count; i++)
		{
			total_size += get_slab_size(&declarations[i]);
		}
		auto *allocator = static_cast<SlabAllocator *>(mem_arena_designate(arena, AllocatorType::Slab, total_size));

		if (allocator == nullptr)
			return nullptr;

		allocator->type          = AllocatorType::Slab;
		allocator->num_slabs     = region_count;
		allocator->slabs         = reinterpret_cast<Slab **>(allocator + 1);
		allocator->max_slab_size = declarations[region_count - 1].block_size;
		auto *slab_pos           = reinterpret_cast<Slab *>(reinterpret_cast<byte *>(allocator) + header_size);
		for (u32 i = 0; i < region_count; i++)
		{
			allocator->slabs[i] = slab_pos;
			slab_pos            = reinterpret_cast<Slab *>(init_slab(allocator->slabs[i], &declarations[i]));
		}
		return allocator;
	}

	static bool slab_has_free_block(Slab *slab)
	{
		for (u32 i = 0; i < slab->free_blocks_len; i++)
		{
			if (slab->free_blocks[i] != U64Max)
				return false;
		}
		return true;
	}

	static u8 index_least_sig_bit(u64 v)
	{
		// I can't be bothered to figure out how or why this works but for those interested: https://stackoverflow.com/a/757266
		static const u8 multiply_de_bruijin_bit_position[32] = {0, 1, 28, 2, 29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4, 8, 31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6, 11, 5, 10, 9};
		return multiply_de_bruijin_bit_position[((u32)((v & -v) * 0x077CB531U)) >> 27];
	}

	static void *slab_alloc(Slab *slab)
	{
		for (u32 i = 0; i < slab->free_blocks_len; i++)
		{
			u64 bitfield;
			do
			{
				// Load the most recent bitfield.
				bitfield    = slab->free_blocks[i].load(std::memory_order_relaxed);
				u64 inverse = ~bitfield;

				// Figure out index.
				u8 bit_index = index_least_sig_bit(inverse);

				// Updated the free blocks.
				u64 mask    = 1 << bit_index;
				u64 new_val = bitfield | mask;

				// If we successfully update the bitfield, then we can assume that we allocated successfully.
				if (slab->free_blocks[i].compare_exchange_weak(bitfield, new_val, std::memory_order_relaxed))
				{
					// Set the index in the entire block array, not just this bitfield.
					return reinterpret_cast<byte *>(slab->heap) + (bit_index + (i * 64)) * slab->block_size;
				}
			} while (bitfield != U64Max);
		}
		return nullptr;
	}

	//	static bool is_allocated(Slab *slab, u32 index)
	//	{
	//		u64 bitfield   = slab->free_blocks[index / 64];
	//		size_t shifted = 1 << (index % 64);
	//		return bitfield & shifted;
	//	}

	static void slab_free(Slab *slab, void *data)
	{
		// TODO(Brandon): Reimplement this check when we actually move blocks in slab reallocation.
		size_t difference = reinterpret_cast<byte *>(data) - reinterpret_cast<byte *>(slab->heap);
		ASSERT(difference % slab->block_size == 0, "Not a valid free address!");

		u32 index          = (reinterpret_cast<byte *>(data) - reinterpret_cast<byte *>(slab->heap)) / slab->block_size;

		u32 bitfield_index = index / 64;
		u64 bitfield       = slab->free_blocks[bitfield_index].load(std::memory_order_relaxed);
		u64 shifted        = 1 << (index % 64);

		ASSERT(bitfield & shifted, "Double free detected!");

		u64 inverse = ~shifted;

		slab->free_blocks[bitfield_index].fetch_and(inverse, std::memory_order_relaxed);
		ASSERT((slab->free_blocks[bitfield_index].load(std::memory_order_relaxed) & shifted) == 0, "Something went wrong writing the correct bitfield");
	}

	void *slab_alloc(SlabAllocator *allocator, size_t size)
	{
		Slab *slab = nullptr;
		ASSERT(size <= allocator->max_slab_size, "Slab allocator does not have a slab of big enough size.");
		for (u32 i = 0; i < allocator->num_slabs; i++)
		{
			if (size <= allocator->slabs[i]->block_size)
			{
				slab = allocator->slabs[i];
				break;
			}
		}
		ASSERT(slab != nullptr, "Failed to find slab!");
		return slab_alloc(slab);
	}

	size_t slab_get_size(SlabAllocator *allocator, void *data)
	{
		for (u32 i = allocator->num_slabs; i > 0; i--)
		{
			if (data > allocator->slabs[i - 1])
				return allocator->slabs[i - 1]->block_size;
		}

		THROW("Failed to find slab for data!");
	}

	void *slab_realloc(SlabAllocator *allocator, void *data, size_t size)
	{
		ASSERT(size <= allocator->max_slab_size, "Slab allocator does not have a slab of big enough size.");
		Slab *cur_slab = nullptr;
		u32 index      = 0;
		for (u32 i = allocator->num_slabs; i > 0; i--)
		{
			if (data > allocator->slabs[i - 1])
			{
				cur_slab = allocator->slabs[i - 1];
				index    = i - 1;
				break;
			}
		}

		ASSERT(cur_slab != nullptr, "Failed to find slab for data!");

		if (size == cur_slab->block_size)
			return data;

		u32 lower_bound = 0;
		u32 upper_bound = allocator->num_slabs;

		if (size > cur_slab->block_size)
			lower_bound = index + 1;
		else // if (size < cur_slab->block_size)
			upper_bound = index;

		for (u32 i = lower_bound; i < upper_bound; i++)
		{
			if (size <= allocator->slabs[i]->block_size)
			{
				void *new_data = slab_alloc(allocator->slabs[i]);

				if (new_data == nullptr)
					return nullptr;

				memmove(new_data, data, cur_slab->block_size);
				slab_free(cur_slab, data);
				return new_data;
			}
		}

		if (size < cur_slab->block_size)
			return data;
		else
			return nullptr;
	}

	void slab_free(SlabAllocator *allocator, void *data)
	{
		Slab *slab = nullptr;
		for (u32 i = allocator->num_slabs; i > 0; i--)
		{
			if (data > allocator->slabs[i - 1])
			{
				slab = allocator->slabs[i - 1];
				break;
			}
		}
		slab_free(slab, data);
	}
} // namespace Vultr