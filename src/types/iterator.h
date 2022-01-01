#pragma once
#include <types/types.h>
#include <types/static_details.h>

namespace Vultr
{
	struct String;
	struct StringView;

	template <typename Container, typename T>
	class Iterator
	{
	  public:
		bool is_end() const { return m_index == Iterator::end(m_container).m_index; }
		size_t index() const { return m_index; }

		bool operator==(Iterator other) const { return m_index == other.m_index; }
		bool operator!=(Iterator other) const { return m_index != other.m_index; }
		bool operator<(Iterator other) const { return m_index < other.m_index; }
		bool operator>(Iterator other) const { return m_index > other.m_index; }
		bool operator<=(Iterator other) const { return m_index <= other.m_index; }
		bool operator>=(Iterator other) const { return m_index >= other.m_index; }

		Iterator operator+(ptrdiff_t delta) const { return Iterator{m_container, m_index + delta}; }
		Iterator operator-(ptrdiff_t delta) const { return Iterator{m_container, m_index - delta}; }

		ptrdiff_t operator-(Iterator other) const { return static_cast<ptrdiff_t>(m_index) - other.m_index; }

		Iterator operator++()
		{
			++m_index;
			return *this;
		}
		Iterator operator++(int)
		{
			++m_index;
			return Iterator(m_container, m_index - 1);
		}

		Iterator operator--()
		{
			--m_index;
			return *this;
		}
		Iterator operator--(int)
		{
			--m_index;
			return Iterator(m_container, m_index + 1);
		}

		const T &operator*() const { return (*m_container)[m_index]; }
		T &operator*() { return (*m_container)[m_index]; }

		auto operator->() const { return m_container + m_index; }
		auto operator->() { return m_container + m_index; }

		Iterator &operator=(const Iterator &other)
		{
			if (this == &other)
			{
				return *this;
			}

			m_index = other.m_index;
			return *this;
		}
		Iterator(const Iterator &obj) = default;

	  private:
		friend Container;
		static Iterator begin(Container *container) { return Iterator(container, 0); }
		static Iterator end(Container *container)
		{
			using raw_type = remove_cv<Container>;
			if constexpr (is_same<raw_type, StringView> || is_same<raw_type, String>)
			{
				return Iterator(container, container->length());
			}
			else
			{
				return Iterator(container, container->size());
			}
		}
		Iterator(Container *container, size_t index) : m_container(container), m_index(index) {}

		Container *m_container = nullptr;
		size_t m_index         = 0;
	};
} // namespace Vultr