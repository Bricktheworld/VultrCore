#pragma once
#include <filesystem/path.h>

namespace Vultr
{
	struct Material
	{
		Path vertex_src;
		Path fragment_src;
	};
} // namespace Vultr