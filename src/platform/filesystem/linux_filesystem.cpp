#include "../platform_impl.h"
#include <sys/stat.h>
#include <errno.h>

namespace Vultr::Platform::Filesystem
{
	ErrorOr<size_t> fsize(str path)
	{
		struct stat st
		{
		};
		if (stat(path, &st) == 0)
		{
			return st.st_size;
		}
		else
		{
			return error_from_errno(errno);
		}
	}

	ErrorOr<bool> is_file(str path)
	{
		auto result = is_directory(path);
		if (result.has_value())
		{
			return !result.value();
		}
		else
		{
			return result.get_error();
		}
	}

	ErrorOr<bool> is_directory(str path)
	{
		struct stat st
		{
		};
		if (stat(path, &st) == 0)
		{
			return S_ISDIR(st.st_mode);
		}
		else
		{
			return error_from_errno(errno);
		}
	}

	size_t path_max() { return PATH_MAX; }

	ErrorOr<void> pwd(char *buf, size_t size)
	{
		if (getcwd(buf, size) != nullptr)
			return Success;

		return error_from_errno(errno);
	}
} // namespace Vultr::Platform::Filesystem
