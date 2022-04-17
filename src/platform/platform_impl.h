#pragma once
#include <types/types.h>
#include <types/error_or.h>
#include <glm/glm.hpp>
#include <types/tuple.h>
#include <thread>
#include <condition_variable>
#include <mutex>

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
		typedef std::mutex Mutex;
		typedef std::condition_variable ConditionVar;
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
		void *get_window_implementation(const Window *window);
		void change_window_mode(Window *window, DisplayMode mode);
		void change_window_monitor(Window *window, Monitor *monitor);
		void change_window_title(Window *window, const char *title);
		f64 update_window(Window *window);
		void close_window(Window *window);
		bool is_focused(Window *window);
		Vec2 get_mouse_pos(Window *window);
		Vec2 get_mouse_delta(Window *window);

		namespace Input
		{
			enum Key : s16
			{
				KEY_SPACE         = 32,
				KEY_APOSTROPHE    = 39,
				KEY_COMMA         = 44,
				KEY_MINUS         = 45,
				KEY_PERIOD        = 46,
				KEY_SLASH         = 47,
				KEY_0             = 48,
				KEY_1             = 49,
				KEY_2             = 50,
				KEY_3             = 51,
				KEY_4             = 52,
				KEY_5             = 53,
				KEY_6             = 54,
				KEY_7             = 55,
				KEY_8             = 56,
				KEY_9             = 57,
				KEY_SEMICOLON     = 59,
				KEY_EQUAL         = 61,
				KEY_A             = 65,
				KEY_B             = 66,
				KEY_C             = 67,
				KEY_D             = 68,
				KEY_E             = 69,
				KEY_F             = 70,
				KEY_G             = 71,
				KEY_H             = 72,
				KEY_I             = 73,
				KEY_J             = 74,
				KEY_K             = 75,
				KEY_L             = 76,
				KEY_M             = 77,
				KEY_N             = 78,
				KEY_O             = 79,
				KEY_P             = 80,
				KEY_Q             = 81,
				KEY_R             = 82,
				KEY_S             = 83,
				KEY_T             = 84,
				KEY_U             = 85,
				KEY_V             = 86,
				KEY_W             = 87,
				KEY_X             = 88,
				KEY_Y             = 89,
				KEY_Z             = 90,
				KEY_LEFT_BRACKET  = 91,
				KEY_BACKSLASH     = 92,
				KEY_RIGHT_BRACKET = 93,
				KEY_GRAVE_ACCENT  = 96,
				KEY_ESCAPE        = 256,
				KEY_ENTER         = 257,
				KEY_TAB           = 258,
				KEY_BACKSPACE     = 259,
				KEY_INSERT        = 260,
				KEY_DELETE        = 261,
				KEY_RIGHT         = 262,
				KEY_LEFT          = 263,
				KEY_DOWN          = 264,
				KEY_UP            = 265,
				KEY_PAGE_UP       = 266,
				KEY_PAGE_DOWN     = 267,
				KEY_HOME          = 268,
				KEY_END           = 269,
				KEY_CAPS_LOCK     = 280,
				KEY_SCROLL_LOCK   = 281,
				KEY_NUM_LOCK      = 282,
				KEY_PRINT_SCREEN  = 283,
				KEY_PAUSE         = 284,
				KEY_F1            = 290,
				KEY_F2            = 291,
				KEY_F3            = 292,
				KEY_F4            = 293,
				KEY_F5            = 294,
				KEY_F6            = 295,
				KEY_F7            = 296,
				KEY_F8            = 297,
				KEY_F9            = 298,
				KEY_F10           = 299,
				KEY_F11           = 300,
				KEY_F12           = 301,
				KEY_F13           = 302,
				KEY_F14           = 303,
				KEY_F15           = 304,
				KEY_F16           = 305,
				KEY_F17           = 306,
				KEY_F18           = 307,
				KEY_F19           = 308,
				KEY_F20           = 309,
				KEY_F21           = 310,
				KEY_F22           = 311,
				KEY_F23           = 312,
				KEY_F24           = 313,
				KEY_F25           = 314,
				KEY_KP_0          = 320,
				KEY_KP_1          = 321,
				KEY_KP_2          = 322,
				KEY_KP_3          = 323,
				KEY_KP_4          = 324,
				KEY_KP_5          = 325,
				KEY_KP_6          = 326,
				KEY_KP_7          = 327,
				KEY_KP_8          = 328,
				KEY_KP_9          = 329,
				KEY_KP_DECIMAL    = 330,
				KEY_KP_DIVIDE     = 331,
				KEY_KP_MULTIPLY   = 332,
				KEY_KP_SUBTRACT   = 333,
				KEY_KP_ADD        = 334,
				KEY_KP_ENTER      = 335,
				KEY_KP_EQUAL      = 336,
				KEY_LEFT_SHIFT    = 340,
				KEY_LEFT_CONTROL  = 341,
				KEY_LEFT_ALT      = 342,
				KEY_LEFT_SUPER    = 343,
				KEY_RIGHT_SHIFT   = 344,
				KEY_RIGHT_CONTROL = 345,
				KEY_RIGHT_ALT     = 346,
				KEY_RIGHT_SUPER   = 347,
				KEY_SHIFT         = 348,
				KEY_CONTROL       = 349,
				KEY_ALT           = 350,
				KEY_SUPER         = 351,
			};

			enum MouseButton : s16
			{
				MOUSE_LEFT     = 0,
				MOUSE_RIGHT    = 1,
				MOUSE_MIDDLE   = 2,
				MOUSE_BUTTON_4 = 3,
				MOUSE_BUTTON_5 = 4,
				MOUSE_BUTTON_6 = 5,
				MOUSE_BUTTON_7 = 6,
				MOUSE_BUTTON_8 = 7,
			};

			enum Action : s16
			{
				RELEASE = 0, // When an input is released
				PRESS   = 1, // When an input is first pressed
				REPEAT  = 2,
			};
		} // namespace Input

		bool key_down(Window *window, Input::Key key);
		bool mouse_down(Window *window, Input::MouseButton button);
		void lock_cursor(Window *window);
		void unlock_cursor(Window *window);

		bool window_should_close(Window *window);
		void swap_buffers(Window *window);
		void poll_events(Window *window);

		u64 current_ms();

		namespace Filesystem
		{
			ErrorOr<size_t> fsize(str path);
			ErrorOr<bool> is_file(str path);
			ErrorOr<bool> is_directory(str path);
			size_t path_max();
			ErrorOr<void> pwd(char *buf, size_t size);
			ErrorOr<u64> fdate_modified_ms(str path);

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
