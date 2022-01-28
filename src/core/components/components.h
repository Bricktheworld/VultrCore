#pragma once
#include "material.h"
#include "mesh.h"
#include "transform.h"

#include <utils/traits.h>
namespace Vultr
{
#define COMPONENT_TRAITS(Type)                                                                                                                                                                                        \
	template <>                                                                                                                                                                                                       \
	struct Traits<Type> : public GenericTraits<Type>                                                                                                                                                                  \
	{                                                                                                                                                                                                                 \
		static consteval StringView type_name() { return #Type; }                                                                                                                                                     \
	}

	COMPONENT_TRAITS(Material);
	COMPONENT_TRAITS(Transform);
	COMPONENT_TRAITS(Mesh);
} // namespace Vultr
