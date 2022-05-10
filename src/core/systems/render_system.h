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
		struct ResizeRequest
		{
			u32 width  = 0;
			u32 height = 0;
		};

		struct Component
		{
			Platform::Framebuffer *output_framebuffer = nullptr;
			Hashmap<void *, Platform::GraphicsPipeline *> pipelines{};
			Queue<void *, 1024> free_queue_listener{};
			Option<ResizeRequest> resize_request = None;
		};

		Component *init();
		void update(const Camera &camera, const Transform &transform, Platform::CmdBuffer *cmd_buffer, Component *system);
		void update(Platform::CmdBuffer *cmd_buffer, Component *system);
		void update(Component *system);
		void request_resize(Component *system, u32 width, u32 height);
		void reinitialize(Component *system, Option<u32> width = None, Option<u32> height = None);
		void destroy(Component *component);
	} // namespace RenderSystem
} // namespace Vultr
