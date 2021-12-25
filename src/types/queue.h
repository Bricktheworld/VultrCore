// TODO(Brandon): Replace with custom allocator.
#pragma once
#include "thread.h"
#include "types.h"

namespace vtl
{
	template <typename T, size_t reserved = 10, u32 growth_numerator = 3, u32 growth_denominator = 2, u32 decay_percent_threshold = 30, bool threaded = false>
	struct Queue
	{
		Queue()
		{
			_size = reserved;
			len   = 0;
			if (reserved > 0)
			{
				// _array = static_cast<T *>(malloc(_size * sizeof(T)));
			}
		}

		~Queue()
		{
			if (_array != nullptr)
			{
				// free(_array);
				_array = nullptr;
			}
		}

		Queue(const Queue &) = delete;
		Queue &operator=(const Queue &) = delete;

		T *front()
		{
			if (threaded)
				queue_mutex.lock();
			assert(len > 0 && "No _array in queue!");
			T *item = &_array[len - 1];
			if (threaded)
				queue_mutex.unlock();
			return item;
		}

		void push(const T &item)
		{
			if (threaded)
				queue_mutex.lock();
			len++;
			_reallocate();
			for (s32 i = len - 1; i >= 1; i--)
			{
				_array[i] = _array[i - 1];
			}

			_array[0] = item;

			if (threaded)
			{
				queue_mutex.unlock();
				queue_cond.notify_one();
			}
		}

		void pop()
		{
			assert(!empty() && "No elements to pop!");
			len--;
			_reallocate();
		}

		void pop_wait()
		{
			assert(threaded && "Queue must be threaded before you can pop wait!");
			std::unique_lock<vtl::mutex> lock(queue_mutex);
			while (empty())
			{
				queue_cond.wait(lock);
			}
			pop();
		}

		bool empty() const { return len == 0; }

		void clear()
		{
			len   = 0;
			_size = reserved;
			if (reserved > 0)
			{
				if (_array == nullptr)
				{
					// _array = (T *)malloc(_size * sizeof(T));
				}
				else
				{
					// _array = (T *)realloc(_array, _size * sizeof(T));
				}
			}
		}

		void _reallocate()
		{
			// Only reallocate and expand the array if we need to
			if (len > _size)
			{
				_size = len * growth_factor;
			}
			else if ((f64)len / (f64)_size < (f64)decay_percent_threshold / 100.0)
			{
				_size = len * growth_factor;
			}
			else
			{
				return;
			}

			if (_size < reserved)
			{
				_size = reserved;
			}

			if (_size == 0)
			{
				if (_array != nullptr)
				{
					// free(_array);
					_array = nullptr;
				}
			}
			else
			{
				if (_array == nullptr)
				{
					_array = (T *)malloc(_size * sizeof(T));
				}
				else
				{
					// _array = (T *)realloc(_array, _size * sizeof(T));
				}
			}
		}

		// The internal array
		T *_array = nullptr;

		// Spaces len (not bytes)
		size_t len = 0;

		// Space in _array (not bytes)
		size_t _size = reserved;

		// To make sure that the buffer expansion is geometric
		f64 growth_factor = (f64)growth_numerator / (f64)growth_denominator;

		vtl::mutex queue_mutex;
		vtl::condition_variable queue_cond;
	};
} // namespace vtl
