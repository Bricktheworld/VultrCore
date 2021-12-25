#pragma once
#include <types/types.h>
#include "file.h"
#include <math/crc32.h>

namespace Vultr
{
	typedef u32 VFileHandle;

	struct InternalVFileStream;
	typedef InternalVFileStream VFileStream;

	// enum struct VFSReadMode
	// {
	//     Read = "r"
	// };

	struct VirtualFilesystem
	{
		// TODO: Reimplement using custom hashtable
		// std::unordered_map<VFileHandle, GenericFile> file_table_path;
		Directory resource_directory;

		VirtualFilesystem(const Directory *asset_directory);
		~VirtualFilesystem();
	};

	s32 vfs_seek(const VirtualFilesystem *vfs, VFileStream *stream, u64 offset);
	bool vfs_file_exists(const VirtualFilesystem *vfs, VFileHandle handle);
	bool vfs_get_file(const VirtualFilesystem *vfs, VFileHandle handle, GenericFile *file);
	u64 vfs_get_file_size(const VirtualFilesystem *vfs, VFileHandle handle);
	u64 vfs_get_file_size(const VirtualFilesystem *vfs, VFileStream *stream);
	VFileStream *vfs_open(const VirtualFilesystem *vfs, VFileHandle handle, const char *mode);
	void vfs_close(VFileStream *stream);
	size_t vfs_read(unsigned char *ptr, size_t size, size_t nmemb, VFileStream *stream);

	unsigned char *vfs_read_full(const VirtualFilesystem *vfs, u64 *size, VFileStream *stream);
	void vfs_free_buf(unsigned char *buf);

	VFileHandle internal_vfile(u32 hash, const char *path, VirtualFilesystem *vfs);
	VFileHandle VFile(const char *path, VirtualFilesystem *vfs);

} // namespace Vultr
