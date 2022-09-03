#if defined _WIN32
// TODO(Brandon): Add entry point if it not compiling shared library.
// #include "entry_point/win32_main.cpp"
#include "memory/win32_memory.cpp"
#include "dynamic_library/win32_dynamic_library.cpp"
#include "window/desktop_window.cpp"
#include "rendering/vulkan/vulkan.cpp"
#include "cli/win32_cli.cpp"
#include "mesh_importing/assimp.cpp"
#include "texture_importing/stbi.cpp"
#elif __linux__
// #include "entry_point/linux_main.cpp"
#include "memory/linux_memory.cpp"
#include "window/desktop_window.cpp"
#include "filesystem/linux_filesystem.cpp"
#include "rendering/vulkan/vulkan.cpp"
#include "cli/linux_cli.cpp"
#include "mesh_importing/assimp.cpp"
#include "texture_importing/stbi.cpp"
#include "time/linux_time.cpp"
#include "uuid/linux_uuid.cpp"
#else
// TODO(Brandon): Determine what needs to be ported to MacOS.
#endif
