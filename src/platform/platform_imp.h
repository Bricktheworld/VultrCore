#pragma once
#if defined _WIN32
#include "threads/win32_threads.h"
#elif __linux__
#include "threads/linux_threads.h"
#else
#endif
