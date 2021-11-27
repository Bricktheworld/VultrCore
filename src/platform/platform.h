#pragma once
#include <types/types.h>

namespace Vultr
{
    namespace Platform
    {
        /**
         * A virtually allocated memory block.
         */
        struct PlatformMemoryBlock;

        /**
         * Gets the real memory pointer from a platform memory block.
         *
         * @param PlatformMemoryBlock *block: The memory block to get the memory from.
         * @return void *: The pointer to the memory.
         *
         * @error Asserts if a nullptr memory block is provided.
         *
         * @thread_safe
         */
        void *get_memory(PlatformMemoryBlock *block);

        /**
         * Get the size of a memory block
         *
         * @param PlatformMemoryBlock *block: The memory block to get the size of.
         * @return size_t: The size of the memory block.
         *
         * @error Asserts if a nullptr memory block is provided.
         *
         * @thread_safe
         */
        size_t get_memory_size(PlatformMemoryBlock *block);

        /**
         * Reserves virtual address space memory from the operating system.
         *
         * @param void *address_hint: The requested address of the memory block. Does not have to be followed.
         * @param size_t size: The size of memory to allocate.
         *
         * @error Will return nullptr if the allocation failed.
         *
         * @thread_safe
         */
        PlatformMemoryBlock *virtual_alloc(void *address_hint, size_t size);

        /**
         * Frees virtual address memory from the operating system.
         *
         * @param PlatformMemoryBlock *block: The block of memory to free.
         *
         * @error Will assert if a nullptr memory block is provided.
         *
         * @thread_safe
         */
        void virtual_free(PlatformMemoryBlock *block);

        /**
         * A struct containing platform thread information.
         */
        struct Thread;

        // TODO(Brandon): Implement threading.
        bool new_thread(Thread *thread);

        /**
         * Load a dynamic library into memory.
         *
         * @param const char *path: The path to the dynamically loaded library. *.so on linux and *.dll on windows.
         *
         * @return void *: The pointer to the DLL.
         *
         * @error Will return nullptr if the library failed to load for whatever reason.
         *
         * @thread_safe
         */
        void *dl_open(const char *path);

        /**
         * Gets the error of the most recent error from a call of a dl function.
         *
         * @return const char *: The human readable error message.
         *
         * @error Returns nullptr if there have been no new errors since last called.
         *
         * @thread_safe
         */
        const char *dl_error();

        /**
         * Closes a dynamic library loaded using dl_open.
         *
         * @param void *dll: The dll loaded through dl_open.
         *
         * @thread_safe
         */
        void dl_close(void *dll);

        /**
         * Loads a symbol in a dynamic library.
         *
         * @param void *dll: The dll loaded through dl_open.
         * @param const char *symbol: The symbol to load. This should be loaded using an unmangled identifier. Usually this can only be done through extern "C"
         *
         * @return void *: The newly loaded symbol.
         *
         * @error Will return nullptr if unable to load this symbol.
         *
         * @thread_safe
         */
        void *dl_load_symbol(void *dll, const char *symbol);

    } // namespace Platform
} // namespace Vultr
