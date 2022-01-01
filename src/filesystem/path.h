#pragma once
#include <types/string.h>
#include <utils/string_utils.h>

namespace Vultr
{
	struct Path
	{
		explicit Path(StringView path) : path(replace_all(path, "\\", "/")) { ASSERT(path != nullptr, "Path must not be null!"); }
		~Path() = default;

		bool is_directory() const { return ends_with(path, '/'); }
		bool is_file() const { return !is_directory(); }

		Path operator/(const Path &other) const
		{
			ASSERT(is_directory(), "Cannot get subdirectory of path that doesn't end with '/'.");
			return Path(StringView(path) + StringView(other.path));
		}

		String path{};
	};
} // namespace Vultr