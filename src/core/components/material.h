#pragma once
#include <filesystem/filesystem.h>
#include <core/resource_allocator/resource_allocator.h>
#include <platform/rendering.h>

namespace Vultr
{
	struct Material
	{
		Resource<Platform::Material *> source{};
	};

} // namespace Vultr