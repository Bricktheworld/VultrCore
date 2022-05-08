#include <vultr_input.h>
#include <vultr.h>

namespace Vultr
{
	namespace Input
	{
		void update_input(InputManager *manager, Vec2 window_offset, Vec2 window_size)
		{

			auto raw_mouse_pos          = Platform::get_mouse_pos(engine()->window);
			auto mapped_mouse           = raw_mouse_pos - window_offset;

			manager->current_mouse_pos  = Vec2(mapped_mouse.x, mapped_mouse.y);

			auto window_dimensions      = Vec2(Platform::get_window_width(engine()->window), Platform::get_window_height(engine()->window));
			auto scaled_raw_mouse_pos   = raw_mouse_pos / window_dimensions;
			manager->mouse_delta        = scaled_raw_mouse_pos - manager->raw_last_mouse_pos;
			manager->raw_last_mouse_pos = scaled_raw_mouse_pos;

			manager->keys               = Platform::get_down_keys(engine()->window);
			manager->mouse_buttons      = Platform::get_mouse_buttons(engine()->window);
			manager->focused            = Platform::is_focused(engine()->window);
			if (!manager->focused)
				manager->mouse_delta = Vec2(0);

			if (manager->mouse_locked)
			{
				if (!Platform::is_cursor_locked(engine()->window))
				{
					manager->mouse_delta = Vec2(0);
					Platform::lock_cursor(engine()->window);
				}
				manager->mouse_on_screen = manager->focused;
			}
			else
			{
				Platform::unlock_cursor(engine()->window);
				manager->mouse_on_screen =
					manager->focused && manager->current_mouse_pos.x >= 0 && manager->current_mouse_pos.y >= 0 && manager->current_mouse_pos.x <= window_size.x && manager->current_mouse_pos.y <= window_size.y;
			}
		}

		void lock_mouse(InputManager *manager) { manager->mouse_locked = true; }
		void unlock_mouse(InputManager *manager) { manager->mouse_locked = false; }
		bool is_mouse_locked(InputManager *manager) { return manager->focused && manager->mouse_locked && Platform::is_cursor_locked(engine()->window); }

		bool mouse_down(const InputManager *manager, const MouseButton &mouse_button) { return manager->mouse_on_screen && (manager->mouse_buttons & mouse_button) == mouse_button; }
		bool mouse_down_strict(const InputManager *manager, const MouseButton &mouse_button) { return manager->mouse_on_screen && manager->mouse_buttons == mouse_button; }

		bool key_down(const InputManager *manager, const Key &key) { return manager->focused && (manager->keys & key) == key; }
		bool key_down_strict(const InputManager *manager, const Key &key) { return manager->focused && manager->keys == key; }

		Vec2 mouse_position(const InputManager *manager) { return manager->current_mouse_pos; }
		Vec2 mouse_delta(const InputManager *manager) { return manager->mouse_delta; }
	} // namespace Input
} // namespace Vultr