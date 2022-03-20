#pragma once
#include <filesystem/filesystem.h>

namespace Vultr
{
	struct Mesh
	{
		Option<Path> source = None;
	};
} // namespace Vultr