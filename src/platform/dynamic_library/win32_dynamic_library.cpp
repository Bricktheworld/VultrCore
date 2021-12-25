#include <types/types.h>
#include "../platform.h"
#include <windows.h>

namespace Vultr
{
	namespace Platform
	{
		void *dl_open(const char *path) { return LoadLibraryA(path); }
		// TODO(Brandon): This is not remotely correct lmao.
		const char *dl_error() { return (const char *)GetLastError(); }
		void dl_close(void *dll) { FreeLibrary((HMODULE)dll); }
		void *dl_load_symbol(void *dll, const char *symbol) { return GetProcAddress((HMODULE)dll, symbol); }
	} // namespace Platform
} // namespace Vultr
