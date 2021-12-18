#if 0
#include "linux_threads.h"

namespace Vultr
{
    namespace Platform
    {
        void join_thread(Thread *thread) { pthread_join(thread->pthread, nullptr); }
        void detach_thread(Thread *thread) { pthread_detach(thread->pthread); }
    } // namespace Platform
} // namespace Vultr
#endif
