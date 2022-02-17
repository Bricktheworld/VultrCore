#pragma once
#include <ecs/entity.h>
#include <platform/rendering.h>

namespace Vultr
{
	namespace ResourceSystem
	{
		struct Component;
	}

	namespace RenderSystem
	{
		struct Component
		{
			Platform::DescriptorLayout *camera_layout = nullptr;
		};

		Component *init();
		void entity_created(void *system, Entity entity);
		void entity_destroyed(void *system, Entity entity);
		void update(Component *system, ResourceSystem::Component *resource_system);
		void destroy(Component *component);
	} // namespace RenderSystem
} // namespace Vultr
