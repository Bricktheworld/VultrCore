#pragma once

namespace Vultr::Platform
{
    typedef struct GLFWmonitor Monitor;
    typedef struct GLFWwindow Window;

    struct WindowMode
    {
        int width;
        int height;
        int red_bits;
        int green_bits;
        int blue_bits;
        int refresh_rate;
    };

    struct WindowHint
    {
        int hint;
        int value;
    };

    struct DisplayMode
    {
        enum Mode
        {
            windowed,
            borderless_windowed,
            fullscreen
        };
    };

    void window_set_context(Window *window);
    void window_set_hint(int hint, int value);
    WindowMode window_get_mode(Monitor *monitor);
    // Window *new_window(DisplayMode::Mode mode, Monitor *monitor, const char *title, std::vector<WindowHint> window_hints);
    // Window *new_window(int width, int height, const char *title, Monitor *monitor, Window *window, std::vector<WindowHint> window_hints);

    bool window_should_close(Window *window);
    void swap_buffers(Window *window);
    void poll_events();
} // namespace Vultr::Platform
