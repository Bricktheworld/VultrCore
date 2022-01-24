#pragma once
#include "entity.h"
#include <core/memory/vultr_memory.h>
#include <types/types.h>

namespace Vultr
{
	struct System
	{
		typedef void (*EntityCreated)(void *component, Entity entity);
		typedef void (*EntityDestroyed)(void *component, Entity entity);

		void *component = nullptr;
		EntityCreated m_created{};
		EntityDestroyed m_destroyed{};
	};

	System new_system(void *component, System::EntityCreated created_cb, System::EntityDestroyed destroyed_cb)
	{
		return {
			.component   = component,
			.m_created   = created_cb,
			.m_destroyed = destroyed_cb,
		};
	}
	void destroy_system(System *system);
} // namespace Vultr