#include <gtest/gtest.h>
#include "vultr_memory.h"

int main(int argc, char **argv)
{
	Vultr::g_game_memory = Vultr::init_game_memory();
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
