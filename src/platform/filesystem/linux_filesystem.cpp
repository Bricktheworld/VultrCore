#include "../platform_impl.h"
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>
#include <utime.h>

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

	ErrorOr<u64> fdate_modified_ms(str path)
	{
		struct stat st;
		if (stat(path, &st) == 0)
		{
			return st.st_mtim.tv_sec * 1e3 + st.st_mtim.tv_nsec * 1e-6;
		}
		else
		{
			return error_from_errno(errno);
		}
	}

	ErrorOr<void> ftouch(str path)
	{
		if (utime(path, nullptr) != 0)
			return error_from_errno(errno);
		return Success;
	}

	ErrorOr<DirectoryHandle *> open_dir(str path)
	{
		DIR *dir = opendir(path);

		if (dir == nullptr)
			return error_from_errno(errno);

		return dir;
	}

	ErrorOr<DirectoryEntry> read_dir(DirectoryHandle *dir)
	{
		while (true)
		{
			struct dirent64 *ent = readdir64(static_cast<DIR *>(dir));
			if (ent == nullptr)
				return Error("No more files to read!");

			ASSERT(ent->d_type != DT_UNKNOWN, "Cannot get directory entry of unknown type!");
			if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0)
				continue;
			switch (ent->d_type)
			{
				case DT_REG:
					return DirectoryEntry{.type = EntryType::FILE, .name = ent->d_name, .uuid = ent->d_ino};
				case DT_DIR:
					return DirectoryEntry{.type = EntryType::DIR, .name = ent->d_name, .uuid = ent->d_ino};
				default:
					continue;
			}
		}
	}

	ErrorOr<void> close_dir(DirectoryHandle *dir)
	{
		closedir(static_cast<DIR *>(dir));
		return Success;
	}
} // namespace Vultr::Platform::Filesystem
