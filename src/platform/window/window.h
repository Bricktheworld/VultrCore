#pragma once
#include <types/types.h>

namespace Vultr::Platform
{
    typedef struct GLFWmonitor Monitor;
    typedef struct GLFWwindow Window;

    struct WindowMode
    {
        u32 width;
        u32 height;
        u16 red_bits;
        u16 green_bits;
        u16 blue_bits;
        u8 refresh_rate;
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
    Window *new_window(DisplayMode::Mode mode, Monitor *monitor, const char *title, WindowHint *window_hints, size_t window_hints_len);
    Window *new_window(int width, int height, const char *title, Monitor *monitor, Window *window, WindowHint *window_hints, size_t window_hints_len);

    bool window_should_close(Window *window);
    void swap_buffers(Window *window);
    void poll_events();
} // namespace Vultr::Platform
