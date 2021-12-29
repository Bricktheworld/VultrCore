#pragma once
#if defined _WIN32
#elif __linux__
#include "threads/linux_threads.h"
#include "dynamic_library/linux_dynamic_library.h"
#else
#endif
