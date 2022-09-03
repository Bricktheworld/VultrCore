#pragma once
#include <ecs/component.h>
#include <filesystem/filesystem.h>
#include <platform/rendering.h>

namespace Vultr
{
	VCOMPONENT_BEGIN(Material)
	VCOMPONENT_FIELD(Resource<Platform::Material *>, source, {})
	VCOMPONENT_FIELD(bool, has_transparency, false)
	VCOMPONENT_END()
} // namespace Vultr
