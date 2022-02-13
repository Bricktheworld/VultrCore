#pragma once
#include "types.h"
#include <vultr_memory.h>

namespace Vultr
{
	template <typename T, size_t inline_capacity = 64>
	struct BufferT
	{
		T *storage = m_inline_storage;

		BufferT()  = default;
		explicit BufferT(size_t size) { resize(size); }
		BufferT(const T *buffer, size_t size) : BufferT(size) { fill(buffer, size); }
		BufferT(const BufferT &other)
		{
			flush();
			resize(other.size());
			memcpy(storage, other.storage, m_size);
		}
		BufferT(BufferT &&other)
		{
			flush();
			m_size   = other.m_size;
			m_inline = other.m_inline;
			if (m_inline)
			{
				memcpy(storage, other.storage, m_size);
			}
			else
			{
				storage = other.storage;
			}
			other.m_size   = 0;
			other.storage  = other.m_inline_storage;
			other.m_inline = true;
		}
		BufferT &operator=(const BufferT &other)
		{
			if (&other == this)
			{
				return *this;
			}
			flush();
			resize(other.size());
			memcpy(storage, other.storage, m_size);
			return *this;
		}
		BufferT &operator=(BufferT &&other)
		{
			if (&other == this)
			{
				return *this;
			}
			flush();
			m_size   = other.m_size;
			m_inline = other.m_inline;
			if (m_inline)
			{
				memcpy(storage, other.storage, m_size);
			}
			else
			{
				storage = other.storage;
			}
			other.m_size   = 0;
			other.storage  = other.m_inline_storage;
			other.m_inline = true;
			return *this;
		}

		T &operator[](size_t index)
		{
			ASSERT(index < m_size, "Cannot get invalid index into buffer!");
			ASSERT(storage != nullptr, "Cannot get invalid index into buffer!");
			return storage[index];
		}

		T &first()
		{
			ASSERT(m_size > 1, "Cannot get first from empty buffer!");
			return storage[0];
		}

		T &last()
		{
			ASSERT(m_size > 1, "Cannot get last from empty buffer!");
			return storage[m_size - 1];
		}

		void resize(size_t new_size)
		{
			if (new_size == 0)
			{
				flush();
			}
			else if (m_size != new_size)
			{
				if (m_inline)
				{
					if (new_size > inline_capacity)
					{
						storage = v_alloc<T>(new_size);
						memcpy(storage, m_inline_storage, m_size);
						m_inline = false;
					}
					m_size = new_size;
				}
				else
				{
					m_size  = new_size;
					storage = v_realloc(storage, m_size);
				}
			}
		}

		void fill(const T *data, size_t len)
		{
			ASSERT(len != 0, "Cannot fill buffer with a length of 0.");
			ASSERT(storage != nullptr, "Cannot fill buffer with invalid source");
			ASSERT(len <= m_size, "Not enough space to fill buffer!");
			memcpy(storage, data, len);
		}

		~BufferT() { flush(); }

		constexpr size_t size() const { return m_size; }
		constexpr bool is_empty() const { return m_size == 0; }

	  protected:
		void flush()
		{
			if (!m_inline && storage != nullptr)
			{
				v_free(storage);
			}
			storage  = m_inline_storage;
			m_size   = 0;
			m_inline = true;
		}

		size_t m_size = 0;

	  private:
		T m_inline_storage[inline_capacity]{};
		bool m_inline = true;
	};

	typedef BufferT<byte> Buffer;
} // namespace Vultr
