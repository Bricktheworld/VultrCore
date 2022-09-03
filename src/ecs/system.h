#pragma once
#include "entity.h"
#include "component.h"
#include <core/memory/memory.h>
#include <types/types.h>

namespace Vultr
{
	struct System
	{
		typedef void (*EntityCreated)(void *component, Entity entity);
		typedef void (*EntityDestroyed)(void *component, Entity entity);

		void *component             = nullptr;
		EntityCreated m_created     = nullptr;
		EntityDestroyed m_destroyed = nullptr;
		Signature component_signature{};
	};

	inline System new_system(void *component, Signature signature, System::EntityCreated created_cb = nullptr, System::EntityDestroyed destroyed_cb = nullptr)
	{
		return {
			.component           = component,
			.m_created           = created_cb,
			.m_destroyed         = destroyed_cb,
			.component_signature = signature,
		};
	}
} // namespace Vultr
