#include "linux_threads.h"

namespace Vultr
{
	namespace Platform
	{
		void join_thread(Thread *thread) { pthread_join(thread->pthread, nullptr); }
		void detach_thread(Thread *thread) { pthread_detach(thread->pthread); }

		Mutex::Mutex()
		{
			pthread_mutexattr_t attr;
			pthread_mutexattr_init(&attr);

			pthread_mutex_init(&pthread_mutex, &attr);

			pthread_mutexattr_destroy(&attr);
		}

		Mutex::~Mutex() { pthread_mutex_destroy(&pthread_mutex); }

		void mutex_lock(Mutex *mutex)
		{
			ASSERT(mutex != nullptr, "Cannot lock nullptr mutex!");
			pthread_mutex_lock(&mutex->pthread_mutex);
		}

		void mutex_unlock(Mutex *mutex)
		{
			ASSERT(mutex != nullptr, "Cannot unlock nullptr mutex!");
			pthread_mutex_unlock(&mutex->pthread_mutex);
		}
	} // namespace Platform
} // namespace Vultr
