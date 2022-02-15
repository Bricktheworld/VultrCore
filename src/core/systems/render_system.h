#pragma once

#include <ecs/entity.h>
#include "resource_system.h"
namespace Vultr
{
	namespace RenderSystem
	{
		struct Component
		{
		};

		Component *init();
		void entity_created(void *system, Entity entity);
		void entity_destroyed(void *system, Entity entity);
		void update(Component *system, ResourceSystem::Component *resource_system);
		void destroy(Component *component);
	} // namespace RenderSystem
} // namespace Vultr
