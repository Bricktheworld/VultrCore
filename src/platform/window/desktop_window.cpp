#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "../platform.h"
#include <core/vultr_core.h>

namespace Vultr::Platform
{

	struct Window
	{
		GLFWwindow *glfw = nullptr;
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

	Window *open_window(LinearAllocator *allocator, DisplayMode mode, Monitor *monitor, const char *title, u32 width, u32 height)
	{
		ASSERT(glfwInit(), "Failed to initialize glfw");
		ASSERT(title != nullptr, "Invalid title.");

		auto *window                 = v_alloc<Window>(allocator);

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

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

		window->glfw = glfwCreateWindow(width, height, title, monitor_param, window_param);
		ASSERT(window->glfw != nullptr, "Failed to create glfw window.");

		if (mode == DisplayMode::WINDOWED)
		{
			glfwMaximizeWindow(window->glfw);
			glfwShowWindow(window->glfw);
		}

		glfwMakeContextCurrent(window->glfw);

		ASSERT(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress), "Failed to load OpenGL using GLAD.");

		return window;
	}

	void close_window(Window *window)
	{
		ASSERT(window != nullptr && window->glfw != nullptr, "Invalid window.");
		glfwDestroyWindow(window->glfw);
		glfwTerminate();
	}

	bool window_should_close(Window *window) { return glfwWindowShouldClose(window->glfw); }

	void swap_buffers(Window *window) { glfwSwapBuffers(window->glfw); }

	void poll_events(Window *window) { glfwPollEvents(); }
} // namespace Vultr::Platform
