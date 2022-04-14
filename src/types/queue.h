#pragma once
#include "types.h"
#include "error_or.h"
#include <platform/platform.h>
#include <utils/transfer.h>

namespace Vultr
{
	template <typename T, ssize_t capacity = 256>
	requires(capacity > 0) struct Queue
	{
		Queue()  = default;
		~Queue() = default;

		Queue(const Queue &other)
		{
			clear();
			memcpy(storage(), other.storage(), capacity * sizeof(T));
			m_front = other.m_front;
			m_rear  = other.m_rear;

			if (!empty())
				queue_cond.notify_one();
		}

		Queue(Queue &&other)
		{
			clear();
			memmove(storage(), other.storage(), capacity * sizeof(T));
			m_front = other.m_front;
			m_rear  = other.m_rear;

			if (!empty())
				queue_cond.notify_one();
		}

		// TODO(Brandon): Fix these move operators because they are not thread safe and they are not correct.
		Queue &operator=(const Queue &other)
		{
			clear();
			memcpy(storage(), other.storage(), capacity * sizeof(T));
			m_front = other.m_front;
			m_rear  = other.m_rear;

			if (!empty())
				queue_cond.notify_one();
			return *this;
		}

		Queue &operator=(Queue &&other)
		{
			clear();
			memmove(storage(), other.storage(), capacity * sizeof(T));
			m_front = other.m_front;
			m_rear  = other.m_rear;

			if (!empty())
				queue_cond.notify_one();
			return *this;
		}

		bool full() const { return (m_front == 0 && m_rear == capacity - 1) || (m_rear == (m_front - 1) % (capacity - 1)); }
		bool empty() const { return m_front == -1; }

		ErrorOr<void> try_push(T &&element)
		{
			{
				Platform::Lock lock(mutex);
				TRY_UNWRAP(auto buf, try_push_impl());
				new (buf) T(move(element));
			}
			queue_cond.notify_one();
			return None;
		}

		void push(T &&element)
		{
			{
				Platform::Lock lock(mutex);
				auto res = try_push_impl();
				ASSERT(res.has_value(), res.get_error().message.c_str());
				new (res.value()) T(move(element));
			}
			queue_cond.notify_one();
		}

		ErrorOr<void> try_push(const T &element)
		{
			{
				Platform::Lock lock(mutex);
				TRY_UNWRAP(auto buf, try_push_impl());
				new (buf) T(element);
			}
			queue_cond.notify_one();
			return None;
		}

		void push(const T &element)
		{
			{
				Platform::Lock lock(mutex);
				auto res = try_push_impl();
				ASSERT(res.has_value(), "%s", res.get_error().message.c_str());
				new (res.value()) T(element);
			}
			queue_cond.notify_one();
		}

		Option<T &> try_front()
		{
			Platform::Lock lock(mutex);
			if (empty())
				return None;

			return storage()[m_front];
		}

		T &front()
		{
			Platform::Lock lock(mutex);
			ASSERT(!empty(), "Queue is empty!");

			return storage()[m_front];
		}

		ErrorOr<T> try_pop()
		{
			Platform::Lock lock(mutex);
			if (empty())
				return Error("Queue is empty!");

			auto data = storage()[m_front];
			storage()[m_front].~T();

			if (m_front == m_rear)
			{
				m_front = -1;
				m_rear  = -1;
			}
			else if (m_front == capacity - 1)
			{
				m_front = 0;
			}
			else
			{
				m_front++;
			}

			return data;
		}

		T pop()
		{
			auto res = try_pop();
			ASSERT(res.has_value(), "%s", res.get_error().message.c_str());
			return res.value();
		}

		T pop_wait()
		{
			Platform::Lock lock(mutex);
			while (empty())
			{
				queue_cond.wait(lock);
			}

			auto data = storage()[m_front];
			storage()[m_front].~T();

			if (m_front == m_rear)
			{
				m_front = -1;
				m_rear  = -1;
			}
			else if (m_front == capacity - 1)
			{
				m_front = 0;
			}
			else
			{
				m_front++;
			}
			return data;
		}

		void clear()
		{
			while (!empty())
			{
				pop();
			}
		}

		T *storage() { return reinterpret_cast<T *>(m_storage); }
		const T *storage() const { return reinterpret_cast<const T *>(m_storage); }

		bool contains(const T &needle) const
		{
			ssize_t front = m_front;
			while (front != -1 && front != m_rear)
			{
				if (storage()[front] == needle)
				{
					return true;
				}
				if (front == capacity - 1)
				{
					front = 0;
				}
				else
				{
					front++;
				}
			}
			return false;
		}

	  private:
		alignas(T) byte m_storage[capacity * sizeof(T)]{};
		s64 m_front = -1;
		s64 m_rear  = -1;
		Platform::ConditionVar queue_cond{};
		Platform::Mutex mutex{};

		ErrorOr<T *> try_push_impl()
		{
			if (full())
				return Error("Queue is full!");

			if (m_front == -1)
			{
				m_front = 0;
				m_rear  = 0;
			}
			else if (m_rear == capacity - 1 && m_front != 0)
			{
				m_rear = 0;
			}
			else
			{
				m_rear++;
			}
			return &storage()[m_rear];
		}
	};
} // namespace Vultr
