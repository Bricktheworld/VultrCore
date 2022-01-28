#pragma once
#include <filesystem/path.h>

namespace Vultr
{
	struct Material
	{
		Option<Path> vertex_src;
		Option<Path> fragment_src;
	};

} // namespace Vultr