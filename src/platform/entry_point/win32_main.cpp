#include <types/types.h>

#include <platform/platform.h>

namespace Vultr
{
	namespace Platform
	{
		struct EntryArgs
		{
			LPSTR p_cmd_line;
			HINSTANCE h_instance;
		};
	} // namespace Platform
} // namespace Vultr

using namespace Vultr;
int CALLBACK WinMain(HINSTANCE instance, HINSTANCE prev_instance, LPSTR command_line, int show_command)
{
	Platform::EntryArgs args = {.p_cmd_line = command_line, .h_instance = instance};
	return Vultr::vultr_main(&args);
}
