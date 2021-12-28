#pragma once
#include <types/string.h>

namespace Vultr
{
	struct Path
	{
		explicit Path(const char *path) : path(path) { ASSERT(path != nullptr, "Path must not be null!"); }
		~Path() = default;

		bool is_directory() const {}

		String path;
	};
} // namespace Vultr