#pragma once
#include <ecs/entity.h>
#include <platform/rendering.h>
#include <core/components/camera.h>
#include <core/components/transform.h>

namespace Vultr
{
	namespace RenderSystem
	{
		struct Component
		{
			Platform::DescriptorLayout *camera_layout   = nullptr;
			Platform::DescriptorLayout *material_layout = nullptr;
			Platform::Framebuffer *output_framebuffer   = nullptr;
			Platform::GraphicsPipeline *pipeline        = nullptr;
		};

		Component *init(const Path &build_path);
		void entity_created(void *system, Entity entity);
		void entity_destroyed(void *system, Entity entity);
		void update(const Camera &camera, const Transform &transform, Platform::CmdBuffer *cmd_buffer, Component *system);
		void update(Platform::CmdBuffer *cmd_buffer, Component *system);
		void update(Component *system);
		void reinitialize(Component *system);
		void destroy(Component *component);
	} // namespace RenderSystem
} // namespace Vultr
