#pragma once
#include <types/types.h>
#include <string.h>
#include <utils/traits.h>

namespace Vultr
{
	static constexpr size_t string_length(str string)
	{
		u32 len = 0;
		if (string)
		{
			while (*string++)
			{
				len++;
			}
		}
		return len;
	}

	struct StringView
	{
	  public:
		constexpr StringView() = default;
		constexpr StringView(const char *ref) : StringView(ref, ref != nullptr ? string_length(ref) : 0) {}
		constexpr StringView(str ref, size_t len) : ref(ref), len(len) {}

		constexpr bool is_empty() const { return len == 0; }
		constexpr operator str() const { return ref; }

		constexpr bool operator==(const StringView &other) const
		{
			if (length() != other.length())
				return false;
			return *this == other.ref;
		}
		constexpr bool operator==(str other) const
		{
			if (other == nullptr && ref == nullptr)
				return true;

			if (ref == nullptr)
				return false;

			if (other == nullptr)
				return false;

			for (size_t i = 0; i < length(); i++)
			{
				ASSERT(ref[i] != '\0', "StringView has incorrect length set!");

				if (other[i] == '\0')
					return false;

				if (other[i] != ref[i])
					return false;
			}
			return true;
		}

		constexpr char operator[](size_t index) const
		{
			ASSERT(index <= len, "Index out of bounds!");

			// Convenient null terminator.
			if (index == len)
				return '\0';
			return ref[index];
		}

		constexpr u32 hash() const
		{
			if (is_empty())
				return 0;
			return string_hash(c_str(), length());
		}

		constexpr size_t length() const { return len; }

		constexpr str c_str() const { return ref; }
		constexpr const StringView substr(size_t start) const
		{
			ASSERT(start < len, "Start index is greater than the length of the string.");
			if (len == 0)
				return StringView();
			return StringView(ref + start, len - start);
		}
		constexpr const StringView substr(size_t start, size_t end) const
		{
			ASSERT(start < len, "Start index is greater than the length of the string.");
			ASSERT(end >= start, "End index is before start index of the string.");
			ASSERT(end <= len, "End index is greater than the length of the string.");
			if (len == 0 || end == start)
				return StringView();
			return StringView(ref + start, end - start);
		}

		constexpr char last() const
		{
			ASSERT(length() != 0, "Cannot get last of empty string!");
			return c_str()[length() - 1];
		}

	  private:
		size_t len = 0;
		str ref    = nullptr;
	};

	template <>
	struct Traits<StringView> : public GenericTraits<StringView>
	{
		static consteval u32 hash(str s, size_t n) { return StringView(s, n).hash(); }
		static constexpr u32 hash(const StringView &s) { return s.hash(); }
		static constexpr u32 equals(const StringView &a, const StringView &b) { return a == b; }
		static constexpr u32 equals(const StringView &a, str b) { return a == b; }
	};
} // namespace Vultr