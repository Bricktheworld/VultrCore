#include "pool.h"

namespace Vultr
{
    struct PoolMemoryBlock
    {
        PoolMemoryBlock *next = nullptr;
    };

    struct PoolSegment
    {
        u32 size                   = 0;
        u32 count                  = 0;
        PoolMemoryBlock *free_head = nullptr;
    };

    static void init_pool_segment(PoolSegment *segment) {}

    PoolAllocator *init_pool_allocator(MemoryArena *arena, u32 allocation_size, u32 count)
    {
        ASSERT(count > 0, "Cannot create a pool allocator with 0 blocks");
        ASSERT(allocation_size > sizeof(PoolMemoryBlock), "Minimum size of a chunk not reached!");

        // Calculate the total space in bytes this allocator will have.
        size_t size = allocation_size * count;

        // Designate a region within the memory arena for our allocator.
        auto *allocator = static_cast<PoolAllocator *>(mem_arena_designate(arena, AllocatorType::Pool, sizeof(PoolAllocator) + sizeof(PoolSegment) + size));

        // If we were unable to allocate the required size, then there is nothing to do.
        if (allocator == nullptr)
            return nullptr;

        allocator->segments              = reinterpret_cast<PoolSegment *>(allocator + 1);
        allocator->num_segments          = 1;
        allocator->segments[0].size      = allocation_size;
        allocator->segments[0].count     = count;
        allocator->segments[0].free_head = reinterpret_cast<PoolMemoryBlock *>(&allocator->segments[0] + 1);

        PoolMemoryBlock *current = allocator->segments[0].free_head;

        for (size_t i = 0; i < count; i++)
        {
            if (i == count - 1)
            {
                current->next = nullptr;
            }
            else
            {
                current->next = reinterpret_cast<PoolMemoryBlock *>(reinterpret_cast<byte *>(current) + allocation_size);
            }
            current = current->next;
        }

        return allocator;
    }

    PoolAllocator *init_pool_allocator(MemoryArena *arena, PoolRegion *regions, u32 region_count)
    {
        // TODO(Brandon): Sort the regions so that allocations are faster and less fragmented.
        size_t size = 0;
        for (s32 i = 0; i < region_count; i++)
        {
            auto *region = &regions[i];

            ASSERT(region->size > sizeof(PoolMemoryBlock), "Minimum size of a chunk not reached!");

            size += region->size * region->count;
        }

        // Designate a region within the memory arena for our allocator.
        auto *allocator = static_cast<PoolAllocator *>(mem_arena_designate(arena, AllocatorType::Pool, size + sizeof(PoolAllocator) + (sizeof(PoolSegment) * region_count)));

        // If we were unable to allocate the required size, then there is nothing to do.
        if (allocator == nullptr)
            return nullptr;

        allocator->segments              = reinterpret_cast<PoolSegment *>(allocator + 1);
        allocator->num_segments          = region_count;
        allocator->segments[0].free_head = reinterpret_cast<PoolMemoryBlock *>(allocator->segments + 1);

        PoolMemoryBlock *current = allocator->segments[0].free_head;
        for (s32 i = 0; i < region_count; i++)
        {
            allocator->segments[i].size      = regions[i].size;
            allocator->segments[i].count     = regions[i].count;
            allocator->segments[i].free_head = reinterpret_cast<PoolMemoryBlock *>(current);

            for (size_t j = 0; j < allocator->segments[0].count; j++)
            {
                if (j == allocator->segments[0].count - 1)
                {
                    current->next = nullptr;
                }
                else
                {
                    current->next = current + 1;
                }
                current++;
            }
        }

        return allocator;
    }

    void *pool_alloc(PoolAllocator *allocator, size_t size)
    {
        for (s32 i = 0; i < allocator->num_segments; i++)
        {
            // If the requested allocation size can't fit here, then we need to find another region.
            if (size > allocator->segments[i].size)
            {
                continue;
            }

            auto *segment = &allocator->segments[i];

            // If the segment doesn't have anymore blocks to allocate, then we need to find another segment.
            if (segment->free_head == nullptr)
            {
                continue;
            }

            auto *data         = segment->free_head;
            segment->free_head = data->next;
            return data;
        }
        return nullptr;
    }

    static PoolSegment *get_segment(PoolAllocator *allocator, void *data)
    {
        for (s32 i = 0; i < allocator->num_segments; i++)
        {
            auto *segment = &allocator->segments[i];

            // If the segment pointer is inside the range of this segment, then we can assume that this data belongs to this segment.
            if (data > segment && data <= reinterpret_cast<byte *>(segment) + segment->count * segment->size)
            {
                return segment;
            }
        }
        return nullptr;
    }

    void *pool_realloc(PoolAllocator *allocator, void *data, size_t size)
    {
        auto *segment = get_segment(allocator, data);

        // If this block already has room for this type then we can just return, and there is nothing to do.
        if (size <= segment->size)
        {
            return data;
        }
        // Otherwise we will find a new block to fit this data and copy the memory there.
        else
        {
            // Find a new memory block.
            void *new_data = pool_alloc(allocator, size);

            // If none was found, then there simply isn't enough memory in this allocator to complete, so we will do nothing.
            if (new_data == nullptr)
            {
                return nullptr;
            }
            else
            {
                // Copy the old bytes to the new location.
                memcpy(new_data, data, segment->size);

                // Free the memory block of the old location.
                auto *memory_block = reinterpret_cast<PoolMemoryBlock *>(data);
                memory_block->next = segment->free_head;
                segment->free_head = memory_block;

                // Return the data.
                return new_data;
            }
        }
    }

    // TODO(Brandon): Add double free detection, because right now that is not implemented!
    void pool_free(PoolAllocator *allocator, void *data)
    {
        auto *segment = get_segment(allocator, data);
        ASSERT(segment != nullptr, "Failed to find segment for requested free from pool allocator!");

        auto *memory_block = reinterpret_cast<PoolMemoryBlock *>(data);
        memory_block->next = segment->free_head;
        segment->free_head = memory_block;
    }
} // namespace Vultr
