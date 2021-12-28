#include <types/types.h>
#include "../platform_impl.h"
#include <dlfcn.h>

namespace Vultr
{
	namespace Platform
	{
		void *dl_open(const char *path) { return dlopen(path, RTLD_NOW); }
		const char *dl_error() { return dlerror(); }
		void dl_close(void *dll) { dlclose(dll); }
		void *dl_load_symbol(void *dll, const char *symbol) { return dlsym(dll, symbol); }
	} // namespace Platform
} // namespace Vultr
