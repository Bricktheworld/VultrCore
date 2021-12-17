#include <platform/platform.h>
#include <core/vultr_core.h>

int Vultr::vultr_main(Platform::EntryArgs *args)
{
    auto *arena = init_mem_arena(Gigabyte(1));
    auto *la    = init_linear_allocator(arena, Kilobyte(1));

    auto *window = Platform::open_window(la, Platform::DisplayMode::BORDERLESS_WINDOWED, nullptr, "Vultr Game Engine");
    while (!Platform::window_should_close(window))
    {
        Platform::poll_events(window);
    }
    Platform::close_window(window);

    linear_free(la);

    return 0;
}
