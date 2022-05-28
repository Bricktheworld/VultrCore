#pragma once
#include "types.h"
#include <utils/traits.h>

namespace Vultr
{
	struct UUID
	{
		bool operator==(const UUID &other) const { return memcmp(m_uuid, other.m_uuid, sizeof(m_uuid)) == 0; }

		u64 m_uuid[2]{};
	};

	template <>
	struct Traits<UUID> : GenericTraits<UUID>
	{
		static constexpr u32 hash(const UUID &uuid) { return string_hash((char *)uuid.m_uuid, sizeof(uuid.m_uuid)); }
	};
} // namespace Vultr