#pragma once
#include "buffer.h"

namespace Vultr
{
	struct String : public BufferT<char>
	{
		String() : BufferT<char>() {}
		String(str string, size_t length, Allocator *allocator = nullptr) : BufferT<char>(string, length + 1, allocator) {}
		String(str string, Allocator *allocator = nullptr) : BufferT<char>(string, strlen(string) + 1, allocator) {}

		String &operator=(str other)
		{
			size_t len = strlen(other) + 1;
			resize(len);
			fill(other, len);

			return *this;
		}

		String &operator+=(str other)
		{
			concat(other);
			return *this;
		}

		String &operator+(str other)
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

		bool operator==(const String &other) const { return *this == other.buffer; }

		bool operator==(str other) const
		{
			if (other == nullptr && buffer == nullptr)
				return true;

			if (buffer == nullptr)
				return false;

			if (other == nullptr)
				return false;

			return strcmp(other, buffer) == 0;
		}

		operator str() { return buffer; }

	  private:
		void concat(str other)
		{
			auto len     = strlen(other) + 1;
			auto old_len = size + 1;
			resize(len + size);
			memcpy(buffer + old_len, other, len);
		}
	};
} // namespace Vultr