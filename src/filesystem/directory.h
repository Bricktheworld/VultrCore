#pragma once
#include <string>
// #include "file.h"
#include <types/types.h>

namespace Vultr
{
	struct IFile;

	struct Directory
	{
		Directory() = default;
		Directory(const char *path);
		Directory(const Directory *dir, const char *path);
		~Directory();

		void operator=(const Directory &other);
		char *path   = nullptr;
	};

	const char *dirbasename(const Directory *dir, size_t *len);

	bool dircurrentworking(Directory *dir);
	bool dirchangeworking(const Directory *dir);

	bool dirhasparent(const Directory *dir);
	void dirparent(const Directory *dir, Directory *parent);
	void dirparent(const IFile *file, Directory *parent);

	bool dirmake(const Directory *dir);
	bool dirmake(const char *path, Directory *dir);

	bool dirremove(const Directory *dir);
	bool dirrename(Directory *dir, const char *name);

	bool dirmove(Directory *src, const Directory *dest);
	bool dirmove(Directory *src, const char *dest);

	bool dircopy(Directory *src, const char *dest);

	bool direxists(const Directory *dir);

	u32 dirfilecount(const Directory *dir);
	u32 dirsubdirectorycount(const Directory *dir);

	Directory *dirsubdirs(const Directory *dir, size_t *len);
} // namespace Vultr
