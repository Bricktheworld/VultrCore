#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/bitfield.h>

using namespace Vultr;
TEST(Bitfield, Create)
{
	{
		Bitfield<10> field;
		ASSERT_EQ(field.m_num_elements, 1);
	}
	{
		Bitfield<64> field;
		ASSERT_EQ(field.m_num_elements, 1);
	}
	{
		Bitfield<128> field;
		ASSERT_EQ(field.m_num_elements, 2);
	}
}

TEST(Bitfield, Set)
{
	{
		Bitfield<10> field;
		field.set(2, true);
		ASSERT_EQ(field.m_elements[0], 0x4);
	}
	{
		Bitfield<64> field;
		field.set(2, true);
		ASSERT_EQ(field.m_elements[0], 0x4);
	}
	{
		Bitfield<128> field;
		field.set(2, true);
		ASSERT_EQ(field.m_elements[0], 0x4);
		field.set(64 + 2, true);
		ASSERT_EQ(field.m_elements[1], 0x4);
	}
	{
		Bitfield<64> field;
		field.set(0, true);
		ASSERT_EQ(field.m_elements[0], 0x1);
		field.set(0, false);
		ASSERT_EQ(field.m_elements[0], 0x0);
	}
}
