#ifdef _WIN32
// TODO(Brandon): Add entry point if it not compiling shared library.
// #include "entry_point/win32_main.cpp"
#include "memory/win32_memory.cpp"
#elif __linux__
// #include "entry_point/linux_main.cpp"
#include "memory/linux_memory.cpp"
#else
// TODO(Brandon): Determine what needs to be ported to MacOS.
#endif
