#include <gtest/gtest.h>
#define private public
#define protected public
#include <types/buffer.h>

using namespace Vultr;
TEST(Buffer, Creation)
{
	{
		Buffer buffer(200);
		ASSERT_NE(buffer.buffer, nullptr);
		ASSERT_EQ(buffer.size, 200);
		buffer[0] = 'c';
		ASSERT_EQ(buffer[0], 'c');
	}
	Buffer third_buffer;
	{
		byte temp_buf[] = {'c', 'a', 't'};
		Buffer buffer(temp_buf, 3);
		ASSERT_NE(buffer.buffer, nullptr);
		ASSERT_EQ(buffer.size, 3);

		buffer[0] = 'c';
		ASSERT_EQ(buffer[0], 'c');
		buffer[1] = 'a';
		ASSERT_EQ(buffer[1], 'a');
		buffer[2] = 't';
		ASSERT_EQ(buffer[2], 't');

		Buffer second_buffer(buffer);
		ASSERT_NE(second_buffer.buffer, buffer.buffer);
		ASSERT_NE(second_buffer.buffer, nullptr);
		ASSERT_EQ(second_buffer.size, buffer.size);
		ASSERT_EQ(second_buffer[0], 'c');
		ASSERT_EQ(second_buffer[1], 'a');
		ASSERT_EQ(second_buffer[2], 't');

		ASSERT_EQ(third_buffer.buffer, nullptr);
		ASSERT_EQ(third_buffer.size, 0);
		third_buffer = second_buffer;
		ASSERT_NE(third_buffer.buffer, second_buffer.buffer);
		ASSERT_NE(third_buffer.buffer, nullptr);
		ASSERT_EQ(third_buffer.size, buffer.size);
	}
	ASSERT_EQ(third_buffer[0], 'c');
	ASSERT_EQ(third_buffer[1], 'a');
	ASSERT_EQ(third_buffer[2], 't');
}

TEST(Buffer, Resize)
{
	Buffer buffer;
	buffer.resize(20);
	ASSERT_EQ(buffer.size, 20);
	ASSERT_NE(buffer.buffer, nullptr);

	byte temp_buf[] = {'c', 'a', 't'};
	buffer.fill(temp_buf, 3);
	ASSERT_EQ(buffer[0], 'c');
	ASSERT_EQ(buffer[1], 'a');
	ASSERT_EQ(buffer[2], 't');

	buffer.resize(0);
	ASSERT_EQ(buffer.size, 0);
	ASSERT_EQ(buffer.buffer, nullptr);
}