#include "../platform_impl.h"
#include <dlfcn.h>

namespace Vultr
{
	namespace Platform
	{
		struct DLL
		{
			void *shared_library = nullptr;
		};

		inline ErrorOr<DLL> dl_open(const char *path)
		{
			DLL dll{};
			auto res = dlopen(path, RTLD_NOW);
			if (res == nullptr)
			{
				return Error(dlerror());
			}
			else
			{
				dll.shared_library = res;
				return dll;
			}
		}
		inline void dl_close(DLL *dll)
		{
			ASSERT(dll != nullptr, "Cannot close nullptr dll.");
			dlclose(dll->shared_library);
		}

		template <typename T>
		ErrorOr<T> dl_load_symbol(DLL *dll, const char *symbol)
		{
			ASSERT(dll != nullptr, "Cannot load symbol from nullptr dll.");
			auto res = dlsym(dll->shared_library, symbol);
			if (res == nullptr)
			{
				return Error(dlerror());
			}
			else
			{
				return (T)(res);
			}
		}
	} // namespace Platform
} // namespace Vultr
