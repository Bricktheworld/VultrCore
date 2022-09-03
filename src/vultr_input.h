#pragma once
#include "input/input.h"
#include "vultr_memory.h"

namespace Vultr
{
	namespace Input
	{
		inline InputManager *input_manager()
		{
			ASSERT(g_game_memory != nullptr && g_game_memory->input_manager != nullptr, "Game memory not properly initialized!");
			return static_cast<InputManager *>(g_game_memory->input_manager);
		}

		inline void lock_mouse() { lock_mouse(input_manager()); }
		inline void unlock_mouse() { unlock_mouse(input_manager()); }
		inline bool is_mouse_locked() { return is_mouse_locked(input_manager()); }

		inline bool mouse_down(const MouseButton &mouse_button) { return mouse_down(input_manager(), mouse_button); }
		inline bool mouse_down_strict(const MouseButton &mouse_button) { return mouse_down_strict(input_manager(), mouse_button); }

		inline bool key_down(const Key &key) { return key_down(input_manager(), key); }
		inline bool key_down_strict(const Key &key) { return key_down_strict(input_manager(), key); }

		inline Vec2 mouse_position() { return mouse_position(input_manager()); }
		inline Vec2 mouse_delta() { return mouse_delta(input_manager()); }
	} // namespace Input
} // namespace Vultr