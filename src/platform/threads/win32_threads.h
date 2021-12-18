#if 0
#pragma once
#include "../platform.h"
#include <types/tuple.h>
#include <pthread.h>

namespace Vultr
{
    namespace Platform
    {
        struct Thread
        {
            pthread_t pthread;
        };

        template <typename ReturnT, typename... Args>
        struct ThreadArgs
        {
            ReturnT (*entry_point)(Args...);
            ReturnT *return_ptr;
            vtl::Tuple<Args...> args;

            ThreadArgs(ReturnT (*entry_point)(Args...), ReturnT *return_ptr, Args... args) : entry_point(entry_point), return_ptr(return_ptr), args(args...) {}
        };

        namespace Internal
        {

            template <typename ReturnT, typename... Args>
            void *thread_entry_point(void *args)
            {
                auto *runnable        = static_cast<ThreadArgs<ReturnT, Args...> *>(args);
                *runnable->return_ptr = runnable->args.apply(runnable->entry_point);
                return nullptr;
            }
        } // namespace Internal

        template <typename ReturnT, typename... Args>
        Thread new_thread(ThreadArgs<ReturnT, Args...> *runnable)
        {
            Thread thread;
            pthread_create(&thread.pthread, nullptr, &Internal::thread_entry_point<ReturnT, Args...>, runnable);
            return thread;
        }

        void join_thread(Thread *thread);

        template <typename ReturnT, typename... Args>
        Thread jthread(ReturnT (*entry_point)(Args...), ReturnT *return_ptr, Args... args)
        {
            auto runnable = ThreadArgs(entry_point, return_ptr, args...);

            auto thread = new_thread(&runnable);

            join_thread(&thread);

            return thread;
        }

        void detach_thread(Thread *thread);

        template <typename ReturnT, typename... Args>
        Thread dthread(ReturnT (*entry_point)(Args...), ReturnT *return_ptr, Args... args)
        {
            auto runnable = ThreadArgs(entry_point, return_ptr, args...);

            auto thread = new_thread(&runnable);

            detach_thread(&thread);

            return thread;
        }

    } // namespace Platform
} // namespace Vultr
#endif
