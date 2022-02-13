#include "render_system.h"
#include <vultr.h>
#include <platform/rendering.h>

namespace Vultr
{
	namespace RenderSystem
	{
		static Component *component(void *component) { return static_cast<Component *>(component); }
		Component *init()
		{
			auto *c        = v_alloc<Component>();
			auto signature = signature_from_components<Transform, Mesh>();
			register_system(c, signature, entity_created, entity_destroyed);
			return c;
		}
		void entity_created(void *system, Entity entity) {}
		void entity_destroyed(void *system, Entity entity) {}
		void update(Component *system)
		{
			if check (Platform::begin_cmd_buffer(engine()->window), auto *cmd, auto _)
			{
				for (auto [entity, transform, mesh] : get_entities<Transform, Mesh>())
				{
				}
				Platform::end_cmd_buffer(cmd);
			}
			else
			{
			}
		}
		void destroy(Component *c) { v_free(c); }
	} // namespace RenderSystem
} // namespace Vultr
