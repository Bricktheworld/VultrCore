#pragma once
#include <types/types.h>
#include <string.h>

namespace Vultr
{
	struct StringView
	{
	  public:
		StringView() = default;
		StringView(str ref) : StringView(ref, ref != nullptr ? strlen(ref) : 0) {}
		StringView(str ref, size_t len) : ref(ref), len(len) {}

		bool is_empty() const { return len == 0; }
		operator str() const { return ref; }

		bool operator==(const StringView &other) const { return *this == other.ref; }
		bool operator==(str other) const
		{
			if (other == nullptr && ref == nullptr)
				return true;

			if (ref == nullptr)
				return false;

			if (other == nullptr)
				return false;

			return strcmp(other, ref) == 0;
		}

		char operator[](size_t index) const
		{
			ASSERT(index <= len, "Index out of bounds!");

			// Convenient null terminator.
			if (index == len)
				return '\0';
			return ref[index];
		}

		size_t length() const { return len; }

		str c_str() const { return ref; }
		const StringView substr(size_t start) const
		{
			ASSERT(start < len, "Start index is greater than the length of the string.");
			if (len == 0)
				return StringView();
			return StringView(ref + start, len - start);
		}
		const StringView substr(size_t start, size_t end) const
		{
			ASSERT(start < len, "Start index is greater than the length of the string.");
			ASSERT(end >= start, "End index is before start index of the string.");
			ASSERT(end <= len, "End index is greater than the length of the string.");
			if (len == 0 || end == start)
				return StringView();
			return StringView(ref + start, end - start);
		}

	  private:
		size_t len = 0;
		str ref    = nullptr;
	};
} // namespace Vultr