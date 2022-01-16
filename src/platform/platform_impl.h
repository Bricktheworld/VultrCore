#pragma once
#include <types/types.h>
#include <types/error_or.h>

namespace Vultr
{
	struct LinearAllocator;
	namespace Platform
	{
		struct EntryArgs;

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

		/**
		 * A struct containing platform mutex information.
		 */
		struct Mutex;

		/**
		 * Lock mutex and wait
		 *
		 * @param Mutex *mutex: The mutex to lock and wait.
		 */
		void mutex_lock(Mutex *mutex);

		/**
		 * Unlock mutex.
		 *
		 * @param Mutex *mutex: The mutex to unlock.
		 */
		void mutex_unlock(Mutex *mutex);

		/**
		 * A lock that automatically unlocks.
		 */
		struct Lock
		{
			Mutex *mutex = nullptr;
			explicit Lock(Mutex *mutex)
			{
				ASSERT(mutex != nullptr, "Cannot create lock from nullptr mutex.");
				this->mutex = mutex;
				mutex_lock(mutex);
			}
			~Lock() { mutex_unlock(mutex); }
		};

		/**
		 * A struct containing platform sempahore information.
		 */
		struct Semaphore;

		/**
		 * Represents a dynamic library in memory.
		 */
		struct DLL;

		/**
		 * Load a dynamic library into memory.
		 *
		 * @param const char *path: The path to the dynamically loaded library. *.so on linux and *.dll on windows.
		 *
		 * @return ErrorOr<void *>: The pointer to the DLL or an error.
		 *
		 * @error Will return an error with a message if it fails to open.
		 *
		 * @thread_safe
		 */
		ErrorOr<DLL> dl_open(const char *path);

		/**
		 * Closes a dynamic library loaded using dl_open.
		 *
		 * @param void *dll: The dll loaded through dl_open.
		 *
		 * @thread_safe
		 */
		void dl_close(DLL *dll);

		/**
		 * Loads a symbol in a dynamic library.
		 *
		 * @param void *dll: The dll loaded through dl_open.
		 * @param const char *symbol: The symbol to load. This should be loaded using an unmangled identifier. Usually this can only be done through extern "C"
		 *
		 * @return T: The newly loaded symbol.
		 *
		 * @error Will return nullptr if unable to load this symbol.
		 *
		 * @thread_safe
		 */
		template <typename T>
		ErrorOr<T> dl_load_symbol(DLL *dll, const char *symbol);

		enum struct DisplayMode : u8
		{
			WINDOWED            = 0x0,
			BORDERLESS_WINDOWED = 0x1,
			FULLSCREEN          = 0x2
		};

		struct Window;
		struct Monitor;

		Window *open_window(DisplayMode mode, Monitor *monitor, str title, bool debug, u32 width = 0, u32 height = 0);
		StringView get_window_title(const Window *window);
		u32 get_window_width(const Window *window);
		u32 get_window_height(const Window *window);
		void change_window_mode(Window *window, DisplayMode mode);
		void change_window_monitor(Window *window, Monitor *monitor);
		void change_window_title(Window *window, const char *title);
		void close_window(Window *window);

		bool window_should_close(Window *window);
		void swap_buffers(Window *window);
		void poll_events(Window *window);

		struct RenderContext;
		RenderContext *init_render_context(const Window *window, bool debug);
		void destroy_render_context(RenderContext *c);

		namespace Filesystem
		{
			ErrorOr<size_t> fsize(str path);
			ErrorOr<bool> is_file(str path);
			ErrorOr<bool> is_directory(str path);
		} // namespace Filesystem

	} // namespace Platform

	/**
	 * Main entry point for the vultr engine. This is the cross platform method that must be implemented in a source file.
	 */
	int vultr_main(Platform::EntryArgs *args);
} // namespace Vultr
