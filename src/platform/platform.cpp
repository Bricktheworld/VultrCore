#if defined _WIN32
// TODO(Brandon): Add entry point if it not compiling shared library.
// #include "entry_point/win32_main.cpp"
#include "memory/win32_memory.cpp"
#include "dynamic_library/win32_dynamic_library.cpp"
#elif __linux__
// #include "entry_point/linux_main.cpp"
#include "memory/linux_memory.cpp"
#include "dynamic_library/linux_dynamic_library.cpp"
#include "threads/linux_threads.cpp"
#else
// TODO(Brandon): Determine what needs to be ported to MacOS.
#endif
