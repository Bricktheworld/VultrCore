#pragma once
#include "buffer.h"

namespace Vultr
{
	struct String : public BufferT<char>
	{
		String(const str string, size_t length, Allocator *allocator = nullptr) : BufferT<char>(string, length + 1, allocator) {}
		String(const str string, Allocator *allocator = nullptr) : BufferT<char>(string, strlen(string) + 1, allocator) {}

		String &operator+=(const str other)
		{
			concat(other);
			return *this;
		}

		String &operator+(const str other)
		{
			concat(other);
			return *this;
		}

		String &operator+=(const String &other)
		{
			concat(other.buffer);
			return *this;
		}

		String &operator+(const String &other)
		{
			concat(other.buffer);
			return *this;
		}

	  private:
		void concat(const str other)
		{
			auto len     = strlen(other) + 1;
			auto old_len = size + 1;
			resize(len + size);
			memcpy(buffer + old_len, other, len);
		}
	};
} // namespace Vultr