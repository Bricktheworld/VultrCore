#pragma once
#include <filesystem/filesystem.h>
#include <core/resource_manager/resource_manager.h>
#include <platform/rendering.h>

namespace Vultr
{
	struct Material
	{
		Resource<Platform::Shader *> source{};
	};

} // namespace Vultr