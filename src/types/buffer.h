#pragma once
#include "types.h"
#include <vultr.h>

namespace Vultr
{
	template <typename T>
	struct BufferT
	{
		T *buffer            = nullptr;
		size_t size          = 0;
		Allocator *allocator = nullptr;

		BufferT() : allocator(g_game_memory->general_allocator){};
		explicit BufferT(size_t size, Allocator *allocator = nullptr)
		{
			this->allocator = allocator ? allocator : g_game_memory->general_allocator;
			this->size      = size;
			buffer          = v_alloc<T>(this->allocator, size);
		}
		BufferT(const T *buffer, size_t size, Allocator *allocator = nullptr) : BufferT(size, allocator) { fill(buffer, size); }
		BufferT(const BufferT &other)
		{
			if (buffer != nullptr)
			{
				v_free(allocator, buffer);
			}
			size      = other.size;
			allocator = other.allocator;
			buffer    = v_alloc<T>(allocator, size);
			memcpy(buffer, other.buffer, size);
		}
		BufferT &operator=(const BufferT &other)
		{
			if (&other == this)
			{
				return *this;
			}
			if (buffer != nullptr)
			{
				v_free(allocator, buffer);
			}
			size      = other.size;
			allocator = other.allocator;
			buffer    = v_alloc<T>(allocator, size);
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
				if (buffer != nullptr)
				{
					v_free(allocator, buffer);
				}
				buffer = nullptr;
				size   = new_size;
			}
			else if (size != new_size)
			{
				size = new_size;
				if (buffer == nullptr)
				{
					buffer = v_alloc<T>(allocator, size);
				}
				else
				{
					buffer = v_realloc(allocator, buffer, size);
				}
			}
		}

		bool safe_resize(size_t new_size)
		{
			if (new_size == 0)
			{
				if (buffer != nullptr)
				{
					v_free(allocator, buffer);
				}
				buffer = nullptr;
				size   = new_size;
			}
			else if (size == new_size)
			{
				return true;
			}

			void *res = nullptr;
			if (buffer == nullptr)
			{
				res = mem_alloc(allocator, new_size);
			}
			else
			{
				res = static_cast<T *>(mem_realloc(allocator, buffer, new_size));
			}

			if (res == nullptr)
			{
				return false;
			}
			else
			{
				size   = new_size;
				buffer = res;
				return true;
			}
		}

		void fill(const T *data, size_t len)
		{
			ASSERT(len != 0, "Cannot fill buffer with a length of 0.");
			ASSERT(buffer != nullptr, "Cannot fill buffer with invalid source");
			ASSERT(len <= size, "Not enough space to fill buffer!");
			memcpy(buffer, data, len);
		}

		~BufferT()
		{
			if (buffer != nullptr)
			{
				v_free(allocator, buffer);
			}
		}
	};

	typedef BufferT<byte> Buffer;
} // namespace Vultr
