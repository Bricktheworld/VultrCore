#pragma once
#include <types/types.h>
#include <types/error_or.h>
#include <types/uuid.h>
#include <glm/glm.hpp>
#include <types/tuple.h>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <input/input.h>

namespace Vultr
{
	struct LinearAllocator;
	struct Pipeline;
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

		typedef std::thread Thread;
		typedef std::recursive_mutex Mutex;
		typedef std::condition_variable_any ConditionVar;
		typedef std::unique_lock<Mutex> Lock;

		template <size_t max>
		using Semaphore = std::counting_semaphore<max>;

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
		ErrorOr<void> dl_close(DLL *dll);

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

		Input::Key get_down_keys(Window *window);
		Input::MouseButton get_mouse_buttons(Window *window);

		u32 get_window_width(const Window *window);
		u32 get_window_height(const Window *window);
		f32 get_window_scale(const Window *window);
		void *get_window_implementation(const Window *window);
		f64 update_window(Window *window);
		void close_window(Window *window);
		bool is_focused(Window *window);
		Vec2 get_mouse_pos(Window *window);

		typedef void (*KeyInputCallback)(void *data, Input::Key key, Input::Action action, Input::Key modifiers);

		struct CallbackHandle
		{
			CallbackHandle() = default;
			CallbackHandle(KeyInputCallback callback, void *data);
			CallbackHandle(const CallbackHandle &other)
			{
				if (count() != 0)
					decr();

				counter_ptr = other.counter_ptr;
				data        = other.data;
				callback    = other.callback;
				incr();
			}

			CallbackHandle &operator=(const CallbackHandle &other)
			{
				if (&other == this)
					return *this;

				if (count() != 0)
					decr();

				counter_ptr = other.counter_ptr;
				data        = other.data;
				callback    = other.callback;
				incr();
				return *this;
			}

			~CallbackHandle();

			void call(Input::Key key, Input::Action action, Input::Key modifiers)
			{
				ASSERT(count() != 0, "Attempting to call handle that no longer has any references.");
				callback(data, key, action, modifiers);
			}

			u32 count()
			{
				if (counter_ptr == nullptr)
					return 0;
				return *counter_ptr;
			}

		  private:
			void incr() { (*counter_ptr)++; }
			void decr()
			{
				ASSERT(count() != 0, "Decrementing past 0");
				(*counter_ptr)--;
			}
			u32 *counter_ptr          = nullptr;
			KeyInputCallback callback = nullptr;
			void *data                = nullptr;
		};

		void lock_cursor(Window *window);
		void unlock_cursor(Window *window);
		bool is_cursor_locked(Window *window);
		CallbackHandle register_key_callback(Window *window, void *data, KeyInputCallback callback);

		bool window_should_close(Window *window);
		void swap_buffers(Window *window);
		void poll_events(Window *window);

		u64 current_ms();

		typedef char UUID_String[sizeof(UUID::m_uuid) * 2 + 5];
		UUID generate_uuid();
		void stringify_uuid(const UUID &uuid, UUID_String out);
		UUID parse_uuid(const StringView &src);

		namespace Filesystem
		{
			ErrorOr<size_t> fsize(str path);
			ErrorOr<bool> is_file(str path);
			ErrorOr<bool> is_directory(str path);
			size_t path_max();
			ErrorOr<void> pwd(char *buf, size_t size);
			ErrorOr<u64> fdate_modified_ms(str path);
			ErrorOr<void> ftouch(str path);

			typedef void DirectoryHandle;
			ErrorOr<DirectoryHandle *> open_dir(str path);

			enum struct EntryType
			{
				FILE,
				DIR,
			};

			struct DirectoryEntry
			{
				EntryType type = EntryType::FILE;
				char *name     = nullptr;
				u64 uuid       = 0;
			};

			ErrorOr<DirectoryEntry> read_dir(DirectoryHandle *dir);
			ErrorOr<void> close_dir(DirectoryHandle *dir);
		} // namespace Filesystem

	} // namespace Platform

	/**
	 * Main entry point for the vultr engine. This is the cross platform method that must be implemented in a source file.
	 */
	int vultr_main(Platform::EntryArgs *args);
} // namespace Vultr
