#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/uuid.h>
#include <platform/platform.h>

using namespace Vultr;
TEST(UUID, Generate)
{
	{
		UUID uuid   = Platform::generate_uuid();
		UUID uuid_2 = uuid;
		ASSERT_TRUE(uuid == uuid_2);
	}
	{
		UUID uuid   = Platform::generate_uuid();
		UUID uuid_2 = Platform::generate_uuid();
		ASSERT_FALSE(uuid == uuid_2);
	}
	{
		UUID uuid = Platform::generate_uuid();
		Platform::UUID_String string;
		Platform::stringify_uuid(uuid, string);
		UUID parsed = Platform::parse_uuid(string);
		ASSERT_TRUE(uuid == parsed);
	}
}