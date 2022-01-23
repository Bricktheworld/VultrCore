#pragma once
#include <filesystem/path.h>

namespace Vultr
{
	struct Mesh
	{
		Option<Path> source = None;
	};
} // namespace Vultr