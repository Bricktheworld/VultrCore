#include <platform/platform.h>
#include <vultr.h>
#include <glad/glad.h>

int Vultr::vultr_main(Platform::EntryArgs *args)
{
    g_game_memory = init_game_memory();

    auto *window = Platform::open_window(g_game_memory->persistent_storage, Platform::DisplayMode::WINDOWED, nullptr, "Vultr Game Engine");

    auto *dl = Platform::dl_open("C:\\Users\\Brand\\Dev\\VultrSandbox\\out\\build\\x64-Debug\\VultrDemo.dll");
    ASSERT(dl != nullptr, "Failed to load game");

    auto *use_game_memory = (UseGameMemoryApi)(Platform::dl_load_symbol(dl, "use_game_memory"));
    use_game_memory(g_game_memory);

    auto *init = (VultrInitApi)Platform::dl_load_symbol(dl, "vultr_init");

    auto *update = (VultrUpdateApi)Platform::dl_load_symbol(dl, "vultr_update");

    init();

    while (!Platform::window_should_close(window))
    {
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        update();
        Platform::swap_buffers(window);
        Platform::poll_events(window);
    }
    Platform::close_window(window);

    linear_free(g_game_memory->persistent_storage);

    return 0;
}
