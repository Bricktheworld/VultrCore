#pragma once
#include "world.h"
#include <core/components/components.h>
#include <types/types.h>
#include <types/typelist.h>

namespace Vultr
{
	template <typename... Component>
	void register_components()
	{
		using WorldT         = World<Mesh, Transform, Material, Component...>;
		g_game_memory->world = persist_alloc(sizeof(WorldT));
		new (g_game_memory->world) WorldT();
	}
} // namespace Vultr
