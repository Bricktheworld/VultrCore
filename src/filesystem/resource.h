#pragma once
#include <types/types.h>
#include <types/static_details.h>
#include <types/string_view.h>

namespace Vultr
{
	template <typename T>
	requires(is_pointer<T>) struct Resource
	{
		consteval Resource(StringView view) { id = Traits<StringView>::hash(view); }

		u32 id;
	};
} // namespace Vultr
