#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <types/vector.h>
#include <math/clamp.h>
#include <core/vultr_core.h>
#include "../rendering.h"
#include <types/string.h>

namespace Vultr::Platform
{
	struct Window
	{
		GLFWwindow *glfw              = nullptr;
		RenderContext *render_context = nullptr;
		String title{};
		u32 width       = 0;
		u32 height      = 0;
		f64 last_time   = 0;
		f32 scale       = 1;
		s32 cursor_mode = GLFW_CURSOR_NORMAL;
		bool is_focused = true;
		Vector<Input::CallbackHandle> key_input_callbacks{};
	};

	struct Monitor
	{
		GLFWmonitor *glfw = nullptr;
	};

	static GLFWmonitor *get_monitor_or_primary(Monitor *monitor)
	{
		if (monitor != nullptr)
		{
			ASSERT(monitor->glfw != nullptr, "Invalid monitor");
			return monitor->glfw;
		}
		else
		{
			auto *res = glfwGetPrimaryMonitor();
			ASSERT(res != nullptr, "Failed to get primary monitor.");
			return res;
		}
	}

	static void window_resize_callback(GLFWwindow *glfw, int width, int height)
	{
		PRODUCTION_ASSERT(width >= 0 && height >= 0, "Invalid width and height received on resize!");
		auto *window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw));
		framebuffer_resize_callback(window, window->render_context, static_cast<u32>(width), static_cast<u32>(height));
	}

	static void window_focus_callback(GLFWwindow *glfw, int focused)
	{
		auto *window       = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw));
		window->is_focused = focused;
		//		if (focused)
		//			window->last_mouse_pos = get_mouse_pos(window);
	}

	static void window_key_callback(GLFWwindow *glfw, int glfw_key, int glfw_scancode, int glfw_action, int glfw_mods)
	{
		auto *window = reinterpret_cast<Window *>(glfwGetWindowUserPointer(glfw));

		Input::Key key;
		if (glfw_key == GLFW_KEY_LEFT_SHIFT || glfw_key == GLFW_KEY_RIGHT_SHIFT)
			key = Input::KEY_SHIFT;
		else if (glfw_key == GLFW_KEY_LEFT_CONTROL || glfw_key == GLFW_KEY_RIGHT_CONTROL)
			key = Input::KEY_CONTROL;
		else if (glfw_key == GLFW_KEY_LEFT_ALT || glfw_key == GLFW_KEY_RIGHT_ALT)
			key = Input::KEY_ALT;
		else if (glfw_key == GLFW_KEY_LEFT_SUPER || glfw_key == GLFW_KEY_RIGHT_SUPER)
			key = Input::KEY_SUPER;
		else if (glfw_key == GLFW_KEY_CAPS_LOCK)
			key = Input::KEY_CAPS_LOCK;
		else if (glfw_key == GLFW_KEY_SPACE)
			key = Input::KEY_SPACE;
		else if (glfw_key == GLFW_KEY_APOSTROPHE)
			key = Input::KEY_APOSTROPHE;
		else if (glfw_key == GLFW_KEY_COMMA)
			key = Input::KEY_COMMA;
		else if (glfw_key == GLFW_KEY_MINUS)
			key = Input::KEY_MINUS;
		else if (glfw_key == GLFW_KEY_PERIOD)
			key = Input::KEY_PERIOD;
		else if (glfw_key == GLFW_KEY_SLASH)
			key = Input::KEY_SLASH;
		else if (glfw_key == GLFW_KEY_0)
			key = Input::KEY_0;
		else if (glfw_key == GLFW_KEY_1)
			key = Input::KEY_1;
		else if (glfw_key == GLFW_KEY_2)
			key = Input::KEY_2;
		else if (glfw_key == GLFW_KEY_3)
			key = Input::KEY_3;
		else if (glfw_key == GLFW_KEY_4)
			key = Input::KEY_4;
		else if (glfw_key == GLFW_KEY_5)
			key = Input::KEY_5;
		else if (glfw_key == GLFW_KEY_6)
			key = Input::KEY_6;
		else if (glfw_key == GLFW_KEY_7)
			key = Input::KEY_7;
		else if (glfw_key == GLFW_KEY_8)
			key = Input::KEY_8;
		else if (glfw_key == GLFW_KEY_9)
			key = Input::KEY_9;
		else if (glfw_key == GLFW_KEY_SEMICOLON)
			key = Input::KEY_SEMICOLON;
		else if (glfw_key == GLFW_KEY_EQUAL)
			key = Input::KEY_EQUAL;
		else if (glfw_key == GLFW_KEY_A)
			key = Input::KEY_A;
		else if (glfw_key == GLFW_KEY_B)
			key = Input::KEY_B;
		else if (glfw_key == GLFW_KEY_C)
			key = Input::KEY_C;
		else if (glfw_key == GLFW_KEY_D)
			key = Input::KEY_D;
		else if (glfw_key == GLFW_KEY_E)
			key = Input::KEY_E;
		else if (glfw_key == GLFW_KEY_F)
			key = Input::KEY_F;
		else if (glfw_key == GLFW_KEY_G)
			key = Input::KEY_G;
		else if (glfw_key == GLFW_KEY_H)
			key = Input::KEY_H;
		else if (glfw_key == GLFW_KEY_I)
			key = Input::KEY_I;
		else if (glfw_key == GLFW_KEY_J)
			key = Input::KEY_J;
		else if (glfw_key == GLFW_KEY_K)
			key = Input::KEY_K;
		else if (glfw_key == GLFW_KEY_L)
			key = Input::KEY_L;
		else if (glfw_key == GLFW_KEY_M)
			key = Input::KEY_M;
		else if (glfw_key == GLFW_KEY_N)
			key = Input::KEY_N;
		else if (glfw_key == GLFW_KEY_O)
			key = Input::KEY_O;
		else if (glfw_key == GLFW_KEY_P)
			key = Input::KEY_P;
		else if (glfw_key == GLFW_KEY_Q)
			key = Input::KEY_Q;
		else if (glfw_key == GLFW_KEY_R)
			key = Input::KEY_R;
		else if (glfw_key == GLFW_KEY_S)
			key = Input::KEY_S;
		else if (glfw_key == GLFW_KEY_T)
			key = Input::KEY_T;
		else if (glfw_key == GLFW_KEY_U)
			key = Input::KEY_U;
		else if (glfw_key == GLFW_KEY_V)
			key = Input::KEY_V;
		else if (glfw_key == GLFW_KEY_W)
			key = Input::KEY_W;
		else if (glfw_key == GLFW_KEY_X)
			key = Input::KEY_X;
		else if (glfw_key == GLFW_KEY_Y)
			key = Input::KEY_Y;
		else if (glfw_key == GLFW_KEY_Z)
			key = Input::KEY_Z;
		else if (glfw_key == GLFW_KEY_LEFT_BRACKET)
			key = Input::KEY_LEFT_BRACKET;
		else if (glfw_key == GLFW_KEY_BACKSLASH)
			key = Input::KEY_BACKSLASH;
		else if (glfw_key == GLFW_KEY_RIGHT_BRACKET)
			key = Input::KEY_RIGHT_BRACKET;
		else if (glfw_key == GLFW_KEY_GRAVE_ACCENT)
			key = Input::KEY_GRAVE_ACCENT;
		else if (glfw_key == GLFW_KEY_ESCAPE)
			key = Input::KEY_ESCAPE;
		else if (glfw_key == GLFW_KEY_ENTER)
			key = Input::KEY_ENTER;
		else if (glfw_key == GLFW_KEY_TAB)
			key = Input::KEY_TAB;
		else if (glfw_key == GLFW_KEY_BACKSPACE)
			key = Input::KEY_BACKSPACE;
		else if (glfw_key == GLFW_KEY_INSERT)
			key = Input::KEY_INSERT;
		else if (glfw_key == GLFW_KEY_DELETE)
			key = Input::KEY_DELETE;
		else if (glfw_key == GLFW_KEY_RIGHT)
			key = Input::KEY_RIGHT;
		else if (glfw_key == GLFW_KEY_LEFT)
			key = Input::KEY_LEFT;
		else if (glfw_key == GLFW_KEY_DOWN)
			key = Input::KEY_DOWN;
		else if (glfw_key == GLFW_KEY_UP)
			key = Input::KEY_UP;
		else if (glfw_key == GLFW_KEY_PAGE_UP)
			key = Input::KEY_PAGE_UP;
		else if (glfw_key == GLFW_KEY_PAGE_DOWN)
			key = Input::KEY_PAGE_DOWN;
		else if (glfw_key == GLFW_KEY_HOME)
			key = Input::KEY_HOME;
		else if (glfw_key == GLFW_KEY_END)
			key = Input::KEY_END;
		else if (glfw_key == GLFW_KEY_F1)
			key = Input::KEY_F1;
		else if (glfw_key == GLFW_KEY_F2)
			key = Input::KEY_F2;
		else if (glfw_key == GLFW_KEY_F3)
			key = Input::KEY_F3;
		else if (glfw_key == GLFW_KEY_F4)
			key = Input::KEY_F4;
		else if (glfw_key == GLFW_KEY_F5)
			key = Input::KEY_F5;
		else if (glfw_key == GLFW_KEY_F6)
			key = Input::KEY_F6;
		else if (glfw_key == GLFW_KEY_F7)
			key = Input::KEY_F7;
		else if (glfw_key == GLFW_KEY_F8)
			key = Input::KEY_F8;
		else if (glfw_key == GLFW_KEY_F9)
			key = Input::KEY_F9;
		else if (glfw_key == GLFW_KEY_F10)
			key = Input::KEY_F10;
		else if (glfw_key == GLFW_KEY_F11)
			key = Input::KEY_F11;
		else if (glfw_key == GLFW_KEY_F12)
			key = Input::KEY_F12;
		else if (glfw_key == GLFW_KEY_F13)
			key = Input::KEY_F13;
		else if (glfw_key == GLFW_KEY_F14)
			key = Input::KEY_F14;
		else if (glfw_key == GLFW_KEY_F15)
			key = Input::KEY_F15;
		else if (glfw_key == GLFW_KEY_F16)
			key = Input::KEY_F16;
		else if (glfw_key == GLFW_KEY_F17)
			key = Input::KEY_F17;
		else if (glfw_key == GLFW_KEY_F18)
			key = Input::KEY_F18;
		else if (glfw_key == GLFW_KEY_F19)
			key = Input::KEY_F19;
		else if (glfw_key == GLFW_KEY_F20)
			key = Input::KEY_F20;
		else if (glfw_key == GLFW_KEY_F21)
			key = Input::KEY_F21;
		else if (glfw_key == GLFW_KEY_F22)
			key = Input::KEY_F22;
		else if (glfw_key == GLFW_KEY_F23)
			key = Input::KEY_F23;
		else if (glfw_key == GLFW_KEY_F24)
			key = Input::KEY_F24;
		else if (glfw_key == GLFW_KEY_F25)
			key = Input::KEY_F25;

		Input::Action action;
		if (glfw_action == GLFW_PRESS)
			action = Input::ACTION_PRESS;
		else if (glfw_action == GLFW_RELEASE)
			action = Input::ACTION_RELEASE;
		else if (glfw_action == GLFW_REPEAT)
			action = Input::ACTION_REPEAT;

		Input::Key modifiers;
		if (glfw_mods & GLFW_MOD_SHIFT)
			modifiers |= Input::KEY_SHIFT;
		if (glfw_mods & GLFW_MOD_CONTROL)
			modifiers |= Input::KEY_CONTROL;
		if (glfw_mods & GLFW_MOD_ALT)
			modifiers |= Input::KEY_ALT;
		if (glfw_mods & GLFW_MOD_SUPER)
			modifiers |= Input::KEY_SUPER;
		if (glfw_mods & GLFW_MOD_CAPS_LOCK)
			modifiers |= Input::KEY_CAPS_LOCK;

		for (s32 i = window->key_input_callbacks.size() - 1; i >= 0; i--)
		{
			if (window->key_input_callbacks[i].count() == 1)
				window->key_input_callbacks.remove(i);
			else
				window->key_input_callbacks[i].call(key, action, modifiers);
		}
	}

	Window *open_window(DisplayMode mode, Monitor *monitor, const char *title, bool debug, u32 width, u32 height)
	{
		ASSERT(glfwInit(), "Failed to initialize glfw");
		ASSERT(title != nullptr, "Invalid title.");

		auto *window = static_cast<Window *>(persist_alloc(sizeof(Window)));
		PRODUCTION_ASSERT(window != nullptr, "Failed to allocate window!");
		new (window) Window();

		GLFWmonitor *monitor_param     = nullptr;
		GLFWwindow *window_param       = nullptr;

		const auto *monitor_video_mode = glfwGetVideoMode(get_monitor_or_primary(monitor));

		switch (mode)
		{
			case DisplayMode::WINDOWED:
				monitor_param = nullptr;
				window_param  = nullptr;

				if (width == 0 || height == 0)
				{
					width  = monitor_video_mode->width - 500;
					height = monitor_video_mode->height - 500;
				}
				// We will resize the window later so don't do anything just yet.
				glfwWindowHint(GLFW_VISIBLE, 0);

				break;
			case DisplayMode::BORDERLESS_WINDOWED:
				monitor_param = get_monitor_or_primary(monitor);

				glfwWindowHint(GLFW_RED_BITS, monitor_video_mode->redBits);
				glfwWindowHint(GLFW_GREEN_BITS, monitor_video_mode->greenBits);
				glfwWindowHint(GLFW_BLUE_BITS, monitor_video_mode->blueBits);
				glfwWindowHint(GLFW_REFRESH_RATE, monitor_video_mode->refreshRate);

				width  = monitor_video_mode->width;
				height = monitor_video_mode->height;
				break;
			case DisplayMode::FULLSCREEN:
				monitor_param = get_monitor_or_primary(monitor);
				if (width == 0 || height == 0)
				{
					width  = monitor_video_mode->width;
					height = monitor_video_mode->height;
				}
				break;
			default:
				break;
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		window->glfw = glfwCreateWindow(width, height, title, monitor_param, window_param);
		PRODUCTION_ASSERT(window->glfw != nullptr, "Failed to create glfw window.");
		glfwSetWindowUserPointer(window->glfw, window);
		glfwSetFramebufferSizeCallback(window->glfw, window_resize_callback);

		window->title  = String(title);
		window->width  = width;
		window->height = height;

		if (mode == DisplayMode::WINDOWED)
		{
			glfwMaximizeWindow(window->glfw);
			glfwShowWindow(window->glfw);
		}

		glfwMakeContextCurrent(window->glfw);

		window->render_context = init_render_context(window, debug);

		f32 scale_x, scale_y;
		glfwGetWindowContentScale(window->glfw, &scale_x, &scale_y);
		window->scale = max(scale_x, scale_y);

		glfwSetKeyCallback(window->glfw, window_key_callback);

		return window;
	}
	StringView get_window_title(const Window *window) { return window->title; }
	u32 get_window_width(const Window *window) { return window->width; }
	u32 get_window_height(const Window *window) { return window->height; }
	f32 get_window_scale(const Window *window) { return window->scale; }
	RenderContext *get_render_context(const Window *window) { return window->render_context; }
	void *get_window_implementation(const Window *window) { return window->glfw; }

	f64 update_window(Window *window)
	{
		f64 t             = glfwGetTime();
		f64 dt            = t - window->last_time;
		window->last_time = t;
		return dt;
	}

	void close_window(Window *window)
	{
		ASSERT(window != nullptr && window->glfw != nullptr && window->render_context != nullptr, "Invalid window.");

		destroy_render_context(window->render_context);
		glfwDestroyWindow(window->glfw);
		glfwTerminate();
	}

	bool is_focused(Window *window) { return window->is_focused; }
	Vec2 get_mouse_pos(Window *window)
	{
		f64 x, y;
		glfwGetCursorPos(window->glfw, &x, &y);
		return {x, y};
	}

	void lock_cursor(Window *window)
	{
		if (window->cursor_mode != GLFW_CURSOR_DISABLED)
		{
			glfwSetInputMode(window->glfw, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			window->cursor_mode = GLFW_CURSOR_DISABLED;
			//			window->last_mouse_pos = get_mouse_pos(window);
		}
	}
	void unlock_cursor(Window *window)
	{
		if (window->cursor_mode != GLFW_CURSOR_NORMAL)
		{
			glfwSetInputMode(window->glfw, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			window->cursor_mode = GLFW_CURSOR_NORMAL;
			//			window->last_mouse_pos = get_mouse_pos(window);
		}
	}

	bool is_cursor_locked(Window *window) { return window->cursor_mode == GLFW_CURSOR_DISABLED; }

	Input::CallbackHandle register_key_callback(Window *window, void *data, Input::KeyInputCallback callback) { return window->key_input_callbacks.push_back(Input::CallbackHandle(callback, data)); }

	Input::Key get_down_keys(Window *window)
	{
		Input::Key key;
		auto *glfw = window->glfw;
		if (glfwGetKey(glfw, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(glfw, GLFW_KEY_RIGHT_SHIFT))
			key |= Input::KEY_SHIFT;
		if (glfwGetKey(glfw, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(glfw, GLFW_KEY_RIGHT_CONTROL))
			key |= Input::KEY_CONTROL;
		if (glfwGetKey(glfw, GLFW_KEY_LEFT_ALT) || glfwGetKey(glfw, GLFW_KEY_RIGHT_ALT))
			key |= Input::KEY_ALT;
		if (glfwGetKey(glfw, GLFW_KEY_LEFT_SUPER) || glfwGetKey(glfw, GLFW_KEY_RIGHT_SUPER))
			key |= Input::KEY_SUPER;
		if (glfwGetKey(glfw, GLFW_KEY_CAPS_LOCK))
			key |= Input::KEY_CAPS_LOCK;
		if (glfwGetKey(glfw, GLFW_KEY_SPACE))
			key |= Input::KEY_SPACE;
		if (glfwGetKey(glfw, GLFW_KEY_APOSTROPHE))
			key |= Input::KEY_APOSTROPHE;
		if (glfwGetKey(glfw, GLFW_KEY_COMMA))
			key |= Input::KEY_COMMA;
		if (glfwGetKey(glfw, GLFW_KEY_MINUS))
			key |= Input::KEY_MINUS;
		if (glfwGetKey(glfw, GLFW_KEY_PERIOD))
			key |= Input::KEY_PERIOD;
		if (glfwGetKey(glfw, GLFW_KEY_SLASH))
			key |= Input::KEY_SLASH;
		if (glfwGetKey(glfw, GLFW_KEY_0))
			key |= Input::KEY_0;
		if (glfwGetKey(glfw, GLFW_KEY_1))
			key |= Input::KEY_1;
		if (glfwGetKey(glfw, GLFW_KEY_2))
			key |= Input::KEY_2;
		if (glfwGetKey(glfw, GLFW_KEY_3))
			key |= Input::KEY_3;
		if (glfwGetKey(glfw, GLFW_KEY_4))
			key |= Input::KEY_4;
		if (glfwGetKey(glfw, GLFW_KEY_5))
			key |= Input::KEY_5;
		if (glfwGetKey(glfw, GLFW_KEY_6))
			key |= Input::KEY_6;
		if (glfwGetKey(glfw, GLFW_KEY_7))
			key |= Input::KEY_7;
		if (glfwGetKey(glfw, GLFW_KEY_8))
			key |= Input::KEY_8;
		if (glfwGetKey(glfw, GLFW_KEY_9))
			key |= Input::KEY_9;
		if (glfwGetKey(glfw, GLFW_KEY_SEMICOLON))
			key |= Input::KEY_SEMICOLON;
		if (glfwGetKey(glfw, GLFW_KEY_EQUAL))
			key |= Input::KEY_EQUAL;
		if (glfwGetKey(glfw, GLFW_KEY_A))
			key |= Input::KEY_A;
		if (glfwGetKey(glfw, GLFW_KEY_B))
			key |= Input::KEY_B;
		if (glfwGetKey(glfw, GLFW_KEY_C))
			key |= Input::KEY_C;
		if (glfwGetKey(glfw, GLFW_KEY_D))
			key |= Input::KEY_D;
		if (glfwGetKey(glfw, GLFW_KEY_E))
			key |= Input::KEY_E;
		if (glfwGetKey(glfw, GLFW_KEY_F))
			key |= Input::KEY_F;
		if (glfwGetKey(glfw, GLFW_KEY_G))
			key |= Input::KEY_G;
		if (glfwGetKey(glfw, GLFW_KEY_H))
			key |= Input::KEY_H;
		if (glfwGetKey(glfw, GLFW_KEY_I))
			key |= Input::KEY_I;
		if (glfwGetKey(glfw, GLFW_KEY_J))
			key |= Input::KEY_J;
		if (glfwGetKey(glfw, GLFW_KEY_K))
			key |= Input::KEY_K;
		if (glfwGetKey(glfw, GLFW_KEY_L))
			key |= Input::KEY_L;
		if (glfwGetKey(glfw, GLFW_KEY_M))
			key |= Input::KEY_M;
		if (glfwGetKey(glfw, GLFW_KEY_N))
			key |= Input::KEY_N;
		if (glfwGetKey(glfw, GLFW_KEY_O))
			key |= Input::KEY_O;
		if (glfwGetKey(glfw, GLFW_KEY_P))
			key |= Input::KEY_P;
		if (glfwGetKey(glfw, GLFW_KEY_Q))
			key |= Input::KEY_Q;
		if (glfwGetKey(glfw, GLFW_KEY_R))
			key |= Input::KEY_R;
		if (glfwGetKey(glfw, GLFW_KEY_S))
			key |= Input::KEY_S;
		if (glfwGetKey(glfw, GLFW_KEY_T))
			key |= Input::KEY_T;
		if (glfwGetKey(glfw, GLFW_KEY_U))
			key |= Input::KEY_U;
		if (glfwGetKey(glfw, GLFW_KEY_V))
			key |= Input::KEY_V;
		if (glfwGetKey(glfw, GLFW_KEY_W))
			key |= Input::KEY_W;
		if (glfwGetKey(glfw, GLFW_KEY_X))
			key |= Input::KEY_X;
		if (glfwGetKey(glfw, GLFW_KEY_Y))
			key |= Input::KEY_Y;
		if (glfwGetKey(glfw, GLFW_KEY_Z))
			key |= Input::KEY_Z;
		if (glfwGetKey(glfw, GLFW_KEY_LEFT_BRACKET))
			key |= Input::KEY_LEFT_BRACKET;
		if (glfwGetKey(glfw, GLFW_KEY_BACKSLASH))
			key |= Input::KEY_BACKSLASH;
		if (glfwGetKey(glfw, GLFW_KEY_RIGHT_BRACKET))
			key |= Input::KEY_RIGHT_BRACKET;
		if (glfwGetKey(glfw, GLFW_KEY_GRAVE_ACCENT))
			key |= Input::KEY_GRAVE_ACCENT;
		if (glfwGetKey(glfw, GLFW_KEY_ESCAPE))
			key |= Input::KEY_ESCAPE;
		if (glfwGetKey(glfw, GLFW_KEY_ENTER))
			key |= Input::KEY_ENTER;
		if (glfwGetKey(glfw, GLFW_KEY_TAB))
			key |= Input::KEY_TAB;
		if (glfwGetKey(glfw, GLFW_KEY_BACKSPACE))
			key |= Input::KEY_BACKSPACE;
		if (glfwGetKey(glfw, GLFW_KEY_INSERT))
			key |= Input::KEY_INSERT;
		if (glfwGetKey(glfw, GLFW_KEY_DELETE))
			key |= Input::KEY_DELETE;
		if (glfwGetKey(glfw, GLFW_KEY_RIGHT))
			key |= Input::KEY_RIGHT;
		if (glfwGetKey(glfw, GLFW_KEY_LEFT))
			key |= Input::KEY_LEFT;
		if (glfwGetKey(glfw, GLFW_KEY_DOWN))
			key |= Input::KEY_DOWN;
		if (glfwGetKey(glfw, GLFW_KEY_UP))
			key |= Input::KEY_UP;
		if (glfwGetKey(glfw, GLFW_KEY_PAGE_UP))
			key |= Input::KEY_PAGE_UP;
		if (glfwGetKey(glfw, GLFW_KEY_PAGE_DOWN))
			key |= Input::KEY_PAGE_DOWN;
		if (glfwGetKey(glfw, GLFW_KEY_HOME))
			key |= Input::KEY_HOME;
		if (glfwGetKey(glfw, GLFW_KEY_END))
			key |= Input::KEY_END;
		if (glfwGetKey(glfw, GLFW_KEY_F1))
			key |= Input::KEY_F1;
		if (glfwGetKey(glfw, GLFW_KEY_F2))
			key |= Input::KEY_F2;
		if (glfwGetKey(glfw, GLFW_KEY_F3))
			key |= Input::KEY_F3;
		if (glfwGetKey(glfw, GLFW_KEY_F4))
			key |= Input::KEY_F4;
		if (glfwGetKey(glfw, GLFW_KEY_F5))
			key |= Input::KEY_F5;
		if (glfwGetKey(glfw, GLFW_KEY_F6))
			key |= Input::KEY_F6;
		if (glfwGetKey(glfw, GLFW_KEY_F7))
			key |= Input::KEY_F7;
		if (glfwGetKey(glfw, GLFW_KEY_F8))
			key |= Input::KEY_F8;
		if (glfwGetKey(glfw, GLFW_KEY_F9))
			key |= Input::KEY_F9;
		if (glfwGetKey(glfw, GLFW_KEY_F10))
			key |= Input::KEY_F10;
		if (glfwGetKey(glfw, GLFW_KEY_F11))
			key |= Input::KEY_F11;
		if (glfwGetKey(glfw, GLFW_KEY_F12))
			key |= Input::KEY_F12;
		if (glfwGetKey(glfw, GLFW_KEY_F13))
			key |= Input::KEY_F13;
		if (glfwGetKey(glfw, GLFW_KEY_F14))
			key |= Input::KEY_F14;
		if (glfwGetKey(glfw, GLFW_KEY_F15))
			key |= Input::KEY_F15;
		if (glfwGetKey(glfw, GLFW_KEY_F16))
			key |= Input::KEY_F16;
		if (glfwGetKey(glfw, GLFW_KEY_F17))
			key |= Input::KEY_F17;
		if (glfwGetKey(glfw, GLFW_KEY_F18))
			key |= Input::KEY_F18;
		if (glfwGetKey(glfw, GLFW_KEY_F19))
			key |= Input::KEY_F19;
		if (glfwGetKey(glfw, GLFW_KEY_F20))
			key |= Input::KEY_F20;
		if (glfwGetKey(glfw, GLFW_KEY_F21))
			key |= Input::KEY_F21;
		if (glfwGetKey(glfw, GLFW_KEY_F22))
			key |= Input::KEY_F22;
		if (glfwGetKey(glfw, GLFW_KEY_F23))
			key |= Input::KEY_F23;
		if (glfwGetKey(glfw, GLFW_KEY_F24))
			key |= Input::KEY_F24;
		if (glfwGetKey(glfw, GLFW_KEY_F25))
			key |= Input::KEY_F25;
		return key;
	}

	Input::MouseButton get_mouse_buttons(Window *window)
	{
		auto *glfw = window->glfw;
		Input::MouseButton mouse_button;
		if (glfwGetMouseButton(glfw, GLFW_MOUSE_BUTTON_LEFT))
			mouse_button |= Input::MOUSE_LEFT;
		if (glfwGetMouseButton(glfw, GLFW_MOUSE_BUTTON_RIGHT))
			mouse_button |= Input::MOUSE_RIGHT;
		if (glfwGetMouseButton(glfw, GLFW_MOUSE_BUTTON_MIDDLE))
			mouse_button |= Input::MOUSE_MIDDLE;
		return mouse_button;
	}

	bool window_should_close(Window *window) { return glfwWindowShouldClose(window->glfw); }

	void swap_buffers(Window *window) { glfwSwapBuffers(window->glfw); }

	void poll_events(Window *window) { glfwPollEvents(); }
} // namespace Vultr::Platform
