#pragma once
#include "types.h"
#include <vultr.h>

namespace Vultr
{
	template <typename T>
	struct BufferT
	{
		T *buffer            = nullptr;
		Allocator *allocator = nullptr;

		BufferT()            = delete;
		BufferT(size_t size, Allocator *allocator) : size(size), allocator(allocator) { buffer = v_alloc<T>(allocator, size); }
		BufferT(T *buffer, size_t size, Allocator *allocator = nullptr) : buffer(buffer), size(size), allocator(allocator ? allocator : g_game_memory->general_allocator) {}
		BufferT &operator=(const BufferT &other) {

		}

		void resize(size_t size) { buffer = v_realloc(allocator, buffer, size); }

		bool safe_resize(size_t size)
		{
			auto *res = static_cast<T *>(mem_realloc(allocator, buffer, size));
			if (res == nullptr)
				return false;

			buffer = res;
			return true;
		}

		void fill(const T *data, size_t len)
		{
			ASSERT(len <= size, "Not enough space to fill buffer!");
			memcpy(buffer, data, len);
		}

		~BufferT()
		{
			if (buffer != nullptr)
			{
				// TODO(Brandon): Replace with custom allocator.
				v_free(allocator, buffer);
			}
		}

	  protected:
		size_t size = 0;
	};

	typedef BufferT<byte> Buffer;

	inline size_t str_len(const char *string)
	{
		size_t count = 0;
		if (string != nullptr)
		{
			while (*string++)
			{
				count++;
			}
		}
		return count;
	}

	struct String : public BufferT<char>
	{
		String(const char *string, Allocator *allocator = nullptr) : BufferT<char>(str_len(string), allocator) { memcpy(buffer, string, size); }
	};
} // namespace Vultr
