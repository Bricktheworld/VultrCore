#pragma once
#include <types/tuple.h>
#include "entity.h"

namespace Vultr
{
	struct EntityManager
	{
	};

	template <typename... Component>
	struct ComponentManager
	{
		Tuple<Component *...> component_arrays{};
		ComponentManager() {}
	};

	struct SystemManager
	{
	};

	struct World
	{
	};
} // namespace Vultr