#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

namespace vtl
{
    typedef std::thread thread;
    typedef std::mutex mutex;
    typedef std::condition_variable condition_variable;

} // namespace vtl
