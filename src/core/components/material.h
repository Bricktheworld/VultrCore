#pragma once
#include <filesystem/filesystem.h>

namespace Vultr
{
	struct Material
	{
		Option<Path> vertex_src;
		Option<Path> fragment_src;
	};

} // namespace Vultr