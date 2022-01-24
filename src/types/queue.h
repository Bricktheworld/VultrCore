#pragma once
#include "types.h"
#include "error_or.h"

namespace Vultr
{
	template <typename T, ssize_t capacity = 256>
	requires(capacity > 0) struct Queue
	{
		Queue()  = default;
		~Queue() = default;

		bool full() const { return (m_front == 0 && m_rear == capacity - 1) || (m_rear == (m_front - 1) % (capacity - 1)); }
		bool empty() const { return m_front == -1; }

		ErrorOr<void> try_push(T &&element)
		{
			UNWRAP(auto buf, try_push_impl());
			new (buf) T(move(element));
			return None;
		}

		void push(T &&element)
		{
			auto res = try_push_impl();
			ASSERT(res.has_value(), res.get_error().message);
			new (res.value()) T(move(element));
		}

		ErrorOr<void> try_push(const T &element)
		{
			UNWRAP(auto buf, try_push_impl());
			new (buf) T(element);
			return None;
		}

		void push(const T &element)
		{
			auto res = try_push_impl();
			ASSERT(res.has_value(), "%s", res.get_error().message.c_str());
			new (res.value()) T(element);
		}

		T &front()
		{
			ASSERT(!empty(), "Queue is empty!");

			return storage()[m_front];
		}

		ErrorOr<T> try_pop()
		{
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
		ssize_t m_front = -1;
		ssize_t m_rear  = -1;

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
