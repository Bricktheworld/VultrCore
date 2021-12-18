#include <platform/platform.h>
#include <core/vultr_core.h>
#include <glad/glad.h>

int Vultr::vultr_main(Platform::EntryArgs *args)
{
    auto *arena = init_mem_arena(Gigabyte(1));
    auto *la    = init_linear_allocator(arena, Kilobyte(1));

    auto *window = Platform::open_window(la, Platform::DisplayMode::WINDOWED, nullptr, "Vultr Game Engine");
    while (!Platform::window_should_close(window))
    {
        // glViewport(0, 0, 3840, 2160);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        Platform::swap_buffers(window);
        Platform::poll_events(window);
    }
    Platform::close_window(window);

    linear_free(la);

    return 0;
}
