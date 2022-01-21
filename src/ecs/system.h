#pragma once
#include "entity.h"
#include <types/types.h>

namespace Vultr
{
	struct System
	{
		typedef void (*entity_created)(Entity entity);
		typedef void (*entity_destroyed)(Entity entity);
	};

	System new_system(System::entity_created created_cb, System::entity_destroyed destroyed_cb);
	void destroy_system(System *system);
} // namespace Vultr