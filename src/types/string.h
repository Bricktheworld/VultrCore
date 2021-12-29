#pragma once
#include "buffer.h"
#include "optional.h"

namespace Vultr
{
	struct String : public BufferT<char>
	{
		String() : BufferT<char>() {}
		String(str string, size_t length) : BufferT<char>(string, length + 1) {}
		explicit String(str string) : BufferT<char>(string, strlen(string) + 1) {}

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

		String operator+(str other)
		{
			size_t len = strlen(other);
			char buf[size + len];
			memcpy(buf, buffer, size - 1);
			memcpy(buf + size - 1, other, len + 1);
			return {buf, size + len - 1};
		}

		String &operator+=(const String &other)
		{
			concat(other.buffer);
			return *this;
		}

		String operator+(const String &other)
		{
			size_t len = strlen(other.buffer);
			char buf[size + len];
			memcpy(buf, buffer, size - 1);
			memcpy(buf, other.buffer, len + 1);
			return {buf, size + len - 1};
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

		Option<size_t> index_of(str charsequence) const { return None; }

	  private:
		void concat(str other)
		{
			auto len     = strlen(other);
			auto old_len = size - 1;
			resize(len + size);
			memcpy(buffer + old_len, other, len + 1);
		}
	};
} // namespace Vultr