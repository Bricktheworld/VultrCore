#pragma once
#include <ecs/entity.h>
#include <platform/rendering.h>
#include <core/components/camera.h>
#include <core/components/transform.h>
#include <types/queue.h>

namespace Vultr
{
	namespace RenderSystem
	{
		struct Component
		{
			Platform::Framebuffer *output_framebuffer = nullptr;
			Hashmap<ResourceId, Platform::GraphicsPipeline *> pipelines{};
		};

		Component *init();
		void entity_created(void *system, Entity entity);
		void entity_destroyed(void *system, Entity entity);
		void update(const Camera &camera, const Transform &transform, Platform::CmdBuffer *cmd_buffer, Component *system);
		void update(Platform::CmdBuffer *cmd_buffer, Component *system);
		void update(Component *system);
		void reinitialize(Component *system);
		void destroy(Component *component);
	} // namespace RenderSystem
} // namespace Vultr
