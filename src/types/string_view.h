#pragma once
#include "string.h"

namespace Vultr
{
	struct StringView
	{
	  public:
		StringView() = default;
		StringView(str ref) : StringView(ref, ref != nullptr ? strlen(ref) : 0) {}
		StringView(str ref, size_t len) : ref(ref), len(len) {}
		StringView(const String &ref) : ref(ref.buffer), len(ref.size - 1) {}

		bool is_empty() const { return len == 0; }
		operator str() const { return ref; }

		bool operator==(const StringView &other) const { return *this == other.ref; }
		bool operator==(const String &other) const { return *this == other.buffer; }
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
			return ref[index];
		}

		size_t length() const { return len; }

		const char *c_str() const { return ref; }

	  private:
		const size_t len = 0;
		str ref          = nullptr;
	};
} // namespace Vultr