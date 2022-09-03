#if 0
// TODO(Brandon): Replace with custom allocator.
#pragma once
#include <assert.h>
#include <memory>
#include "types/types.h"

namespace vtl
{
	template <typename T, size_t reserved = 10, u32 growth_numerator = 3, u32 growth_denominator = 2, u32 decay_percent_threshold = 30, bool threaded = false>
	struct Stack
	{
		Stack()
		{
			_size = reserved;
			len   = 0;
			if (reserved > 0)
			{
				// _array = static_cast<T *>(malloc(_size * sizeof(T)));
			}
		}

		Stack(T *array, size_t count)
		{
			assert(count != 0 && "Count must be greater than 0!");
			assert(array != nullptr && "Array must not be null!");

			len   = count;
			_size = len * growth_factor;
			if (_size < reserved)
			{
				_size = reserved;
			}

			// _array = static_cast<T *>(malloc(sizeof(T) * _size));

			for (int i = 0; i < count; i++)
			{
				_array[i] = array[i];
			}
		}

		// Delete copy methods because we don't want this to be done on accident, we want it to be very very explicit since we are essentially duplicating a buffer
		Stack<T> &operator=(const Stack<T> &other) = delete;
		Stack(const Stack<T> &other)               = delete;

		// Destructor for dynamic array
		~Stack()
		{
			// if (_array != nullptr)
			//     free(_array);
		}

		// Push element to back of Stack
		// If the Stack does not have enough space, then a reallocation will
		// occur
		//
		// Returns the element just inserted
		T *push(const T &element)
		{
			if (threaded)
				stack_mutex.lock();
			len++;
			_reallocate();

			// Increase the len amount and assign the last element of array to the new
			// element
			_array[len - 1] = element;

			if (threaded)
			{
				stack_mutex.unlock();
				stack_cond.notify_one();
			}

			return &_array[len - 1];
		}

		T *top()
		{
			if (threaded)
				stack_mutex.lock();
			assert(len > 0 && "No _array in queue!");
			T *item = &_array[len - 1];
			if (threaded)
				stack_mutex.unlock();
			return item;
		}

		// Pop last element of stack
		void pop()
		{
			if (threaded)
				std::unique_lock<vtl::mutex> lock(stack_mutex);
			size_t index = len - 1;

			// Fail if the index is greater than the len or negative
			assert(index <= len && index >= 0 && "Index out of bounds!");

			// Shift elements to the left
			for (uint i = index + 1; i < _size; i++)
			{
				_array[i - 1] = _array[i];
			}

			// Decrease len even if no reallocation
			len--;

			_reallocate();
		}

		void pop_wait()
		{
			assert(threaded && "Stack must be threaded before you can pop wait!");
			std::unique_lock<vtl::mutex> lock(stack_mutex);
			while (empty())
			{
				stack_cond.wait(lock);
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

		// Spaces len (not bytes)
		size_t len = 0;

		// Space in _array (not bytes)
		size_t _size = reserved;

		// To make sure that the buffer expansion is geometric
		f64 growth_factor = (f64)growth_numerator / (f64)growth_denominator;

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
					// _array = (T *)malloc(_size * sizeof(T));
				}
				else
				{
					// _array = (T *)realloc(_array, _size * sizeof(T));
				}
			}
		}

		// The internal array
		T *_array;

		vtl::mutex stack_mutex;
		vtl::condition_variable stack_cond;
	};
} // namespace vtl

#endif