#include <GLFW/glfw3.h>
#include <platform/window/window.h>

namespace Vultr::Platform
{
    void window_set_context(Window *window)
    {
        glfwMakeContextCurrent(window);
    }

    void window_set_hint(int hint, int value)
    {
        glfwWindowHint(hint, value);
    }

    WindowMode window_get_mode(Monitor *monitor = glfwGetPrimaryMonitor())
    {
        const GLFWvidmode *mode = glfwGetVideoMode(monitor);
        return {
            .width = mode->width,
            .height = mode->height,
            .red_bits = mode->redBits,
            .green_bits = mode->greenBits,
            .blue_bits = mode->blueBits,
            .refresh_rate = mode->refreshRate,
        };
    }

    // Window *new_window(DisplayMode::Mode display_mode, Monitor *monitor, const char *title, std::vector<WindowHint> window_hints)
    // {
    //     for (auto pair : window_hints)
    //     {
    //         glfwWindowHint(pair.hint, pair.value);
    //     }
    //     auto video_mode = window_get_mode(monitor);
    //     switch (display_mode)
    //     {
    //     case DisplayMode::windowed:
    //         return glfwCreateWindow(video_mode.width, video_mode.height, title, nullptr, nullptr);
    //     case DisplayMode::fullscreen:
    //         return glfwCreateWindow(video_mode.width, video_mode.height, title, monitor, nullptr);
    //     case DisplayMode::borderless_windowed:
    //         glfwWindowHint(GLFW_RED_BITS, video_mode.red_bits);
    //         glfwWindowHint(GLFW_GREEN_BITS, video_mode.green_bits);
    //         glfwWindowHint(GLFW_BLUE_BITS, video_mode.blue_bits);
    //         glfwWindowHint(GLFW_REFRESH_RATE, video_mode.refresh_rate);
    //         return glfwCreateWindow(video_mode.width, video_mode.height, title, monitor, nullptr);
    //     default:
    //         return nullptr;
    //     }
    // }

    // Window *new_window(int width, int height, const char *title, Monitor *monitor, Window *window, std::vector<WindowHint> window_hints);

    bool window_should_close(Window *window)
    {
        return glfwWindowShouldClose(window);
    }

    void swap_buffers(Window *window)
    {
        glfwSwapBuffers(window);
    }

    void poll_events()
    {
        glfwPollEvents();
    }
} // namespace Vultr::Platform
