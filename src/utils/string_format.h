#pragma once
#include <types/string_view.h>

namespace Vultr
{
	template <typename... Args>
	struct StringFormat
	{
		consteval StringFormat(StringView fmt) {}


	};
} // namespace Vultr