#pragma once
#include "types.h"
#include <utils/hash.h>

namespace Vultr
{
	struct StringHash
	{
		template <size_t n>
		consteval StringHash(const char (&string)[n])
		{
			hash = string_hash<n>(string);
			raw  = StringView(string, n);
		}

		constexpr StringView c_str() { return raw; }
		constexpr u32 value() { return hash; }

	  private:
		u32 hash = 0;
		StringView raw;
	};
} // namespace Vultr