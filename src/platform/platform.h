#pragma once
#include "platform_impl.h"
#if defined _WIN32
#elif __linux__
#include "threads/linux_threads.h"
#else
#endif
