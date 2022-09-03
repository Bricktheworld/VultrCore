#include "../platform_impl.h"
#include <uuid/uuid.h>
#include <utils/transfer.h>

namespace Vultr
{
	namespace Platform
	{
		UUID generate_uuid()
		{
			static_assert(sizeof(uuid_t) == sizeof(UUID::m_uuid), "UUID size is not valid!");
			uuid_t uuid;
			uuid_generate(uuid);

			UUID ret{};
			memmove(ret.m_uuid, uuid, sizeof(ret.m_uuid));
			return ret;
		}

		void stringify_uuid(const UUID &uuid, UUID_String out)
		{
			uuid_t uuid_raw;
			memcpy(uuid_raw, uuid.m_uuid, sizeof(uuid.m_uuid));

			uuid_unparse_lower(uuid_raw, out);
		}

		UUID parse_uuid(const StringView &src)
		{
			static_assert(sizeof(uuid_t) == sizeof(UUID::m_uuid), "UUID size is not valid!");

			UUID_String buf{};
			strncpy(buf, src.c_str(), src.length());

			uuid_t uuid;
			uuid_parse(buf, uuid);

			UUID ret{};
			memmove(ret.m_uuid, uuid, sizeof(ret.m_uuid));
			return ret;
		}
	} // namespace Platform
} // namespace Vultr
