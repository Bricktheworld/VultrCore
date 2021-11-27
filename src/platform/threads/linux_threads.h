#pragma once
#include "../platform.h"
#include <pthread.h>

namespace Vultr
{
    namespace Platform
    {
        struct Thread
        {
            pthread_t pthread;
        };

        template <typename F>
        bool new_thread(PoolAllocator *pa, Thread *thread, F entry_point)
        {

            return true;
        }
    } // namespace Platform
} // namespace Vultr
