#pragma once
#include <types/types.h>
#include <types/bitfield.h>
#include <glm/glm.hpp>

namespace Vultr
{
	namespace Input
	{
		typedef Bitfield<128> Key;
		static constexpr Key KEY_SHIFT         = 0;
		static constexpr Key KEY_CONTROL       = 1;
		static constexpr Key KEY_ALT           = 2;
		static constexpr Key KEY_SUPER         = 3;
		static constexpr Key KEY_CAPS_LOCK     = 4;
		static constexpr Key KEY_SPACE         = 5;
		static constexpr Key KEY_APOSTROPHE    = 6;
		static constexpr Key KEY_COMMA         = 7;
		static constexpr Key KEY_MINUS         = 8;
		static constexpr Key KEY_PERIOD        = 9;
		static constexpr Key KEY_SLASH         = 10;
		static constexpr Key KEY_0             = 11;
		static constexpr Key KEY_1             = 12;
		static constexpr Key KEY_2             = 13;
		static constexpr Key KEY_3             = 14;
		static constexpr Key KEY_4             = 15;
		static constexpr Key KEY_5             = 16;
		static constexpr Key KEY_6             = 17;
		static constexpr Key KEY_7             = 18;
		static constexpr Key KEY_8             = 19;
		static constexpr Key KEY_9             = 20;
		static constexpr Key KEY_SEMICOLON     = 21;
		static constexpr Key KEY_EQUAL         = 22;
		static constexpr Key KEY_A             = 23;
		static constexpr Key KEY_B             = 24;
		static constexpr Key KEY_C             = 25;
		static constexpr Key KEY_D             = 26;
		static constexpr Key KEY_E             = 27;
		static constexpr Key KEY_F             = 28;
		static constexpr Key KEY_G             = 29;
		static constexpr Key KEY_H             = 30;
		static constexpr Key KEY_I             = 31;
		static constexpr Key KEY_J             = 32;
		static constexpr Key KEY_K             = 33;
		static constexpr Key KEY_L             = 34;
		static constexpr Key KEY_M             = 35;
		static constexpr Key KEY_N             = 36;
		static constexpr Key KEY_O             = 37;
		static constexpr Key KEY_P             = 38;
		static constexpr Key KEY_Q             = 39;
		static constexpr Key KEY_R             = 40;
		static constexpr Key KEY_S             = 41;
		static constexpr Key KEY_T             = 42;
		static constexpr Key KEY_U             = 43;
		static constexpr Key KEY_V             = 44;
		static constexpr Key KEY_W             = 45;
		static constexpr Key KEY_X             = 46;
		static constexpr Key KEY_Y             = 47;
		static constexpr Key KEY_Z             = 48;
		static constexpr Key KEY_LEFT_BRACKET  = 49;
		static constexpr Key KEY_BACKSLASH     = 50;
		static constexpr Key KEY_RIGHT_BRACKET = 51;
		static constexpr Key KEY_GRAVE_ACCENT  = 52;
		static constexpr Key KEY_ESCAPE        = 53;
		static constexpr Key KEY_ENTER         = 54;
		static constexpr Key KEY_TAB           = 55;
		static constexpr Key KEY_BACKSPACE     = 56;
		static constexpr Key KEY_INSERT        = 57;
		static constexpr Key KEY_DELETE        = 58;
		static constexpr Key KEY_RIGHT         = 59;
		static constexpr Key KEY_LEFT          = 60;
		static constexpr Key KEY_DOWN          = 61;
		static constexpr Key KEY_UP            = 62;
		static constexpr Key KEY_PAGE_UP       = 63;
		static constexpr Key KEY_PAGE_DOWN     = 64;
		static constexpr Key KEY_HOME          = 65;
		static constexpr Key KEY_END           = 66;
		static constexpr Key KEY_F1            = 67;
		static constexpr Key KEY_F2            = 68;
		static constexpr Key KEY_F3            = 69;
		static constexpr Key KEY_F4            = 70;
		static constexpr Key KEY_F5            = 71;
		static constexpr Key KEY_F6            = 72;
		static constexpr Key KEY_F7            = 73;
		static constexpr Key KEY_F8            = 74;
		static constexpr Key KEY_F9            = 75;
		static constexpr Key KEY_F10           = 76;
		static constexpr Key KEY_F11           = 77;
		static constexpr Key KEY_F12           = 78;
		static constexpr Key KEY_F13           = 79;
		static constexpr Key KEY_F14           = 80;
		static constexpr Key KEY_F15           = 81;
		static constexpr Key KEY_F16           = 82;
		static constexpr Key KEY_F17           = 83;
		static constexpr Key KEY_F18           = 84;
		static constexpr Key KEY_F19           = 85;
		static constexpr Key KEY_F20           = 86;
		static constexpr Key KEY_F21           = 87;
		static constexpr Key KEY_F22           = 88;
		static constexpr Key KEY_F23           = 89;
		static constexpr Key KEY_F24           = 90;
		static constexpr Key KEY_F25           = 91;

		typedef Bitfield<64> MouseButton;
		static constexpr MouseButton MOUSE_LEFT   = 0;
		static constexpr MouseButton MOUSE_RIGHT  = 1;
		static constexpr MouseButton MOUSE_MIDDLE = 2;

		//		static constexpr Key MOUSE_BUTTON_4 = 3;
		//		static constexpr Key MOUSE_BUTTON_5 = 4;
		//		static constexpr Key MOUSE_BUTTON_6 = 5;
		//		static constexpr Key MOUSE_BUTTON_7 = 6;
		//		static constexpr Key MOUSE_BUTTON_8 = 7;

		typedef u8 Action;
		static constexpr Action ACTION_RELEASE = 0; // When an input is released
		static constexpr Action ACTION_PRESS   = 1; // When an input is down
		static constexpr Action ACTION_REPEAT  = 2;

		struct InputManager
		{
			Key keys{};
			MouseButton mouse_buttons{};

			Vec2 current_mouse_pos  = Vec2(0);
			Vec2 raw_last_mouse_pos = Vec2(0);
			Vec2 mouse_delta        = Vec2(0);
			bool mouse_locked       = false;
			bool mouse_on_screen    = false;
			bool focused            = false;
		};

		void update_input(InputManager *manager, Vec2 window_offset = Vec2(0), Vec2 window_size = Vec2(0));

		void lock_mouse(InputManager *manager);
		void unlock_mouse(InputManager *manager);
		bool is_mouse_locked(InputManager *manager);

		bool mouse_down(const InputManager *manager, const MouseButton &mouse_button);
		bool mouse_down_strict(const InputManager *manager, const MouseButton &mouse_button);

		bool key_down(const InputManager *manager, const Key &key);
		bool key_down_strict(const InputManager *manager, const Key &key);

		Vec2 mouse_position(const InputManager *manager);
		Vec2 mouse_delta(const InputManager *manager);

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

		CallbackHandle register_key_callback(void *data, KeyInputCallback callback);
	} // namespace Input
} // namespace Vultr
