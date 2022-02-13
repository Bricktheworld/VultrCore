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
		u32 width     = 0;
		u32 height    = 0;
		f64 last_time = 0;
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

		if (mode == DisplayMode::WINDOWED)
		{
			glfwMaximizeWindow(window->glfw);
			glfwShowWindow(window->glfw);
		}

		glfwMakeContextCurrent(window->glfw);

		window->title          = String(title);
		window->width          = width;
		window->height         = height;

		window->render_context = init_render_context(window, debug);

		return window;
	}
	StringView get_window_title(const Window *window) { return window->title; }
	u32 get_window_width(const Window *window) { return window->width; }
	u32 get_window_height(const Window *window) { return window->height; }
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

	bool window_should_close(Window *window) { return glfwWindowShouldClose(window->glfw); }

	void swap_buffers(Window *window) { glfwSwapBuffers(window->glfw); }

	void poll_events(Window *window) { glfwPollEvents(); }
} // namespace Vultr::Platform
