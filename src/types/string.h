#pragma once
#include "buffer.h"
#include "optional.h"
#include "string_view.h"
#include "iterator.h"
#include <utils/traits.h>

namespace Vultr
{
	struct String : public BufferT<char>
	{
		String() : BufferT<char>(1) { storage[0] = '\0'; }
		explicit String(StringView string) : BufferT<char>(string.length() + 1)
		{
			if (string.length() > 0)
				fill(string.c_str(), string.length());
			storage[string.length()] = '\0';
			ASSERT(size() > 0, "String buffer size should never be less than 1 because of null terminator.");
		}
		String(str string, size_t length) : String(StringView(string, length)) {}

		String &operator=(StringView other)
		{
			flush();
			size_t size = other.length() + 1;
			resize(size);
			fill(other, size);

			return *this;
		}

		String &operator+=(StringView other)
		{
			concat(other);
			return *this;
		}

		String &operator+=(const String &other)
		{
			concat(StringView(other.storage, other.length()));
			return *this;
		}
		operator StringView() const
		{
			if (is_empty())
				return {};
			return {storage, length()};
		}

		bool operator==(const String &other) const { return *this == other.storage; }
		bool operator==(StringView other) const
		{
			if (other == nullptr && is_empty())
				return true;

			if (is_empty())
				return false;

			if (other == nullptr)
				return false;

			if (other.length() != length())
				return false;

			return strncasecmp(other, storage, other.length()) == 0;
		}
		bool operator==(str other) const { return *this == StringView(other); }

		constexpr u32 hash() const
		{
			if (is_empty())
				return 0;
			return string_hash(c_str(), length());
		}

		str c_str() const { return storage; }
		operator str() { return storage; }
		constexpr size_t length() const { return size() - 1; }
		constexpr size_t is_empty() const { return length() == 0; }

		using SIterator = Iterator<String, char>;
		SIterator begin() { return SIterator::begin(this); }
		SIterator end() { return SIterator::end(this); }

		using SCIterator = Iterator<const String, const char>;
		SCIterator begin() const { return SCIterator::begin(this); }
		SCIterator end() const { return SCIterator::end(this); }

	  private:
		void clear()
		{
			flush();
			m_size     = 1;
			storage[0] = '\0';
		}

		void concat(StringView other)
		{
			if (other == nullptr)
				return;
			auto old_len = size() - 1;
			resize(other.length() + size());
			memcpy(storage + old_len, other, other.length());
			storage[length()] = '\0';
		}
	};

	inline String operator+(StringView a, StringView b)
	{
		auto new_len = a.length() + b.length();
		char buf[new_len + 1];
		memcpy(buf, a.c_str(), a.length());
		memcpy(buf + a.length(), b.c_str(), b.length());
		buf[new_len] = '\0';

		return String(StringView(buf, new_len));
	}

	template <>
	struct Traits<String> : public GenericTraits<String>
	{
		static u32 hash(const String &s) { return s.hash(); }
		static u32 equals(const String &a, const String &b) { return a == b; }
		static u32 equals(const String &a, str b) { return a == b; }
	};
} // namespace Vultr
