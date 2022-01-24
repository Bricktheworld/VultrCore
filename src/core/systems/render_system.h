#pragma once

#include <ecs/entity.h>
namespace Vultr
{
	namespace RenderSystem
	{
		struct Component
		{
		};

		void init();
		void entity_created(void *system, Entity entity);
		void entity_destroyed(void *system, Entity entity);
		void update(Component *system);
	} // namespace RenderSystem
} // namespace Vultr