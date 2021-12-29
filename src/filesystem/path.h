#pragma once
#include <types/string.h>
#include <utils/string_utils.h>

namespace Vultr
{
	struct Path
	{
		explicit Path(const char *path) : path(path) { ASSERT(path != nullptr, "Path must not be null!"); }
		~Path() = default;

		bool is_directory() const { return ends_with(path, '/'); }
		bool is_file() const { return !is_directory(); }

		String path;
	};
} // namespace Vultr