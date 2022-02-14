#pragma once
#include <types/types.h>
#include <string.h>
#include <utils/traits.h>

namespace Vultr
{
	struct StringView
	{
	  public:
		constexpr StringView() = default;
		constexpr StringView(str ref) : StringView(ref, ref != nullptr ? strlen(ref) : 0) {}
		constexpr StringView(str ref, size_t len) : ref(ref), len(len) {}

		constexpr bool is_empty() const { return len == 0; }
		operator str() const { return ref; }

		constexpr bool operator==(const StringView &other) const { return *this == other.ref; }
		constexpr bool operator==(str other) const
		{
			if (other == nullptr && ref == nullptr)
				return true;

			if (ref == nullptr)
				return false;

			if (other == nullptr)
				return false;

			return strcmp(other, ref) == 0;
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
		static u32 hash(const StringView &s) { return s.hash(); }
		static u32 equals(const StringView &a, const StringView &b) { return a == b; }
		static u32 equals(const StringView &a, str b) { return a == b; }
	};

	template <typename T>
	struct ReflTraits : public Traits<T>
	{
		static consteval StringView type_name();
		static consteval u32 type_id() { return string_hash(type_name(), type_name().length()); }
	};
} // namespace Vultr