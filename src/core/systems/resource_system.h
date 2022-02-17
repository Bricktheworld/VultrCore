#pragma once
#include <ecs/entity.h>
#include <types/hashmap.h>
#include <core/components/mesh.h>
#include <platform/rendering.h>

namespace Vultr
{
	namespace RenderSystem
	{
		struct Component;
	}
	namespace ResourceSystem
	{
		struct Component
		{
			Path resource_dir{};
			Platform::UploadContext *upload_context = nullptr;
			Platform::GraphicsPipeline *pipeline    = nullptr;
			Hashmap<u32, Platform::Mesh *> loaded_meshes{};
		};

		Component *init(RenderSystem::Component *render_system, const Path &resource_dir, const Path &build_path);
		void entity_created(void *system, Entity entity);
		void entity_destroyed(void *system, Entity entity);
		void update(Component *system);
		void destroy(Component *component);
	} // namespace ResourceSystem
} // namespace Vultr
