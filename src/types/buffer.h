#pragma once
#include "types.h"
#include <vultr.h>

namespace Vultr
{
	template <typename T>
	struct BufferT
	{
		T *buffer   = nullptr;
		size_t size = 0;

		BufferT()   = default;
		explicit BufferT(size_t size)
		{
			this->size = size;
			buffer     = v_alloc<T>(size);
		}
		BufferT(const T *buffer, size_t size) : BufferT(size) { fill(buffer, size); }
		BufferT(const BufferT &other)
		{
			flush();
			size   = other.size;
			buffer = v_alloc<T>(size);
			memcpy(buffer, other.buffer, size);
		}
		BufferT &operator=(const BufferT &other)
		{
			if (&other == this)
			{
				return *this;
			}
			flush();
			size   = other.size;
			buffer = v_alloc<T>(size);
			memcpy(buffer, other.buffer, size);
			return *this;
		}

		T &operator[](size_t index)
		{
			ASSERT(index < size, "Cannot get invalid index into buffer!");
			ASSERT(buffer != nullptr, "Cannot get invalid index into buffer!");
			return buffer[index];
		}

		void resize(size_t new_size)
		{
			if (new_size == 0)
			{
				flush();
			}
			else if (size != new_size)
			{
				size = new_size;
				if (buffer == nullptr)
				{
					buffer = v_alloc<T>(size);
				}
				else
				{
					buffer = v_realloc(buffer, size);
				}
			}
		}

		void fill(const T *data, size_t len)
		{
			ASSERT(len != 0, "Cannot fill buffer with a length of 0.");
			ASSERT(buffer != nullptr, "Cannot fill buffer with invalid source");
			ASSERT(len <= size, "Not enough space to fill buffer!");
			memcpy(buffer, data, len);
		}

		~BufferT() { flush(); }

	  protected:
		void flush()
		{
			if (buffer != nullptr)
			{
				v_free(buffer);
				buffer = nullptr;
				size   = 0;
			}
		}
	};

	typedef BufferT<byte> Buffer;
} // namespace Vultr
