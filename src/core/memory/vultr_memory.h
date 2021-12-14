#pragma once
#include "vultr_memory_internal.h"
#include "linear.h"
#include "pool.h"
#include "free_list.h"

namespace Vultr
{
    /**
     * Allocate some memory using a memory arena.
     *
     * @param Allocator *allocator: The memory allocator.
     * @param size_t count: The number of blocks of memory of the requested size to allocate.
     *
     * @return T *: The newly allocated memory.
     */
    template <typename T>
    T *malloc(Allocator *allocator, size_t count = 1)
    {
        ASSERT(allocator != nullptr, "Cannot allocate from an invalid memory allocator!");
        switch (allocator->type)
        {
            case AllocatorType::Linear:
                linear_alloc(static_cast<LinearAllocator *>(allocator), sizeof(T));
                break;
            case AllocatorType::Pool:
                pool_alloc(static_cast<PoolAllocator *>(allocator), sizeof(T));
                break;
            case AllocatorType::FreeList:
                free_list_alloc(static_cast<FreeListAllocator *>(allocator), sizeof(T));
                break;
            case AllocatorType::Stack:
                NOT_IMPLEMENTED("Stack has not yet been implemented in the engine :(");
                break;
            case AllocatorType::None:
            default:
                THROW("Invalid memory allocator, how the fuck did you even get here.");
        }
    }

    /**
     * Reallocate a block of memory using an allocator.
     *
     * @param Allocator *allocator: The memory allocator.
     * @param T *memory: The memory that was allocated.
     *
     * @return T *: The reallocated memory.
     *
     * @error This will return nullptr if it failed to reallocate and will leave the original memory untouched.
     */
    template <typename T>
    T *realloc(Allocator *allocator, T *memory)
    {
        switch (allocator->type)
        {
            case AllocatorType::Linear:
                THROW("Cannot reallocate in a linear allocator, the entire point of linear is to not do that.");
                break;
            case AllocatorType::Pool:
                pool_realloc(static_cast<PoolAllocator *>(allocator), memory, sizeof(T));
                break;
            case AllocatorType::FreeList:
                free_list_realloc(static_cast<FreeListAllocator *>(allocator), memory, sizeof(T));
                break;
            case AllocatorType::Stack:
                THROW("Cannot reallocate in a stack allocator, this is not what a stack allocator is for.");
                break;
            case AllocatorType::None:
            default:
                THROW("Invalid memory allocator, how the fuck did you even get here.");
        }
    }

    /**
     * Free a block of memory using an allocator.
     *
     * @param Allocator *allocator: The memory allocator.
     * @param T *memory: The memory that was allocated.
     */
    template <typename T>
    void free(Allocator *allocator, T *memory)
    {
        switch (allocator->type)
        {
            case AllocatorType::Linear:
                THROW("You cannot free individual blocks of memory from a linear allocator.");
                break;
            case AllocatorType::Pool:
                pool_free(static_cast<PoolAllocator *>(allocator), memory);
                break;
            case AllocatorType::FreeList:
                free_list_free(static_cast<FreeListAllocator *>(allocator), memory);
                break;
            case AllocatorType::Stack:
                NOT_IMPLEMENTED("Stack has not yet been implemented in the engine :(");
                break;
            case AllocatorType::None:
            default:
                THROW("Invalid memory allocator, how the fuck did you even get here.");
        }
    }
    /**
     * Class that can be inherited from to show dependency on some external allocator.
     */
    struct MemoryStruct
    {
        MemoryStruct(Allocator *allocator) : allocator(allocator) { ASSERT(allocator != nullptr, "Memory struct must be initalized with a valid memory allocator!"); }
        Allocator *allocator = nullptr;

        /**
         * Shorthand for a memory struct to allocate a block of memory using @ref malloc(Allocator *)
         *
         * @param Allocator *allocator: The memory allocator.
         *
         * @return T *: The newly allocated memory.
         */
        template <typename T>
        T *malloc()
        {
            return malloc<T>(allocator);
        }

        /**
         * Shorthand for a memory struct to reallocate a block of memory using @ref realloc(Allocator *)
         *
         * @param Allocator *allocator: The memory allocator.
         * @param T *memory: The memory that was allocated.
         *
         * @return T *: The reallocated memory.
         *
         * @error This will return nullptr if it failed to reallocate and will leave the original memory untouched.
         */
        template <typename T>
        T *realloc(T *memory)
        {
            return realloc<T>(allocator, memory);
        }

        /**
         * Shorthand for a memory struct to free a block of memory using @ref realloc(Allocator *)
         *
         * @param Allocator *allocator: The memory allocator.
         * @param T *memory: The memory that was allocated.
         */
        template <typename T>
        void free(T *memory)
        {
            free<T>(allocator, memory);
        }
    };
} // namespace Vultr
