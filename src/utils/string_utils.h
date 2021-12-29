#pragma once
#include <types/types.h>
#include <types/string_view.h>

namespace Vultr
{
	enum struct CaseSensitivity
	{
		Insensitive,
		Sensitive
	};

	bool starts_with(StringView haystack, char needle, CaseSensitivity cs = CaseSensitivity::Sensitive);
	bool starts_with(StringView string, StringView start, CaseSensitivity cs = CaseSensitivity::Sensitive);
	bool ends_with(StringView haystack, char needle, CaseSensitivity cs = CaseSensitivity::Sensitive);
	bool ends_with(StringView string, StringView end, CaseSensitivity cs = CaseSensitivity::Sensitive);
	bool contains(StringView haystack, char needle, CaseSensitivity cs = CaseSensitivity::Sensitive);
	bool contains(StringView haystack, StringView needle, CaseSensitivity cs = CaseSensitivity::Sensitive);
	Option<size_t> find(StringView haystack, char needle, size_t start = 0);
	Option<size_t> find(StringView haystack, StringView needle, size_t start = 0);
	Option<size_t> find_last(StringView haystack, char needle);

} // namespace Vultr