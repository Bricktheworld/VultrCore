// TODO: Reimplement String to be more safe
#pragma once

#define _FILE_OFFSET_BITS 64
#include <types/types.h>
#include <cstring>
// #include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "directory.h"

namespace Vultr
{
#define EXTENSION_MAX 255

	struct IFile
	{
		char *path                                                         = nullptr;

		virtual const char *const *get_file_extensions(size_t *size) const = 0;
		virtual char *expected_extensions_string()                         = 0;
	};

	template <const char *const extensions[]>
	struct File : IFile
	{
		static constexpr size_t extensions_length = sizeof(extensions) / sizeof(char *const);

		File() { path = nullptr; }

		File(const char *path)
		{
			size_t len = strlen(path);
			assert(path[len - 1] != '/' && "File ends with a `/`!");

			// this->path = str(path);

			// strcreplace(this->path, '\\', '/');
		}

		File(const Directory *dir, const char *fname)
		{
			size_t flen = strlen(fname);
			size_t plen = strlen(dir->path);

			assert(fname[flen - 1] != '/' && "File ends with a `/`!");
			assert(fname[0] != '/' && "File name cannot start with a `/`!");

			// path = str(dir->path);
			// path = strappend(path, fname);

			// strcreplace(path, '\\', '/');
		}

		File(const File &other)
		{
			// path = str(other.path);
		}

		~File()
		{
			// TODO(Brandon): Reimplement.
			// if (path != nullptr)
			//     free(path);
			path = nullptr;
		}

		const char *const *get_file_extensions(size_t *size) const override
		{
			*size = extensions_length;
			return extensions;
		}

		char *expected_extensions_string() override
		{
			// char *expected_extensions = str(100);
			// for (size_t i = 0; i < extensions_length; i++)
			// {
			//     strcat(expected_extensions, extensions[i]);
			// }
			// return expected_extensions;
		}

		void operator=(const File &other)
		{
			// TODO(Brandon): Reimplement.
			// if (path != nullptr)
			// free(path);
			// path = str(other.path);
		}
	};

	namespace FileTypes
	{
		// OK who in the c++ standard committee thought that these syntax being valid was a good idea
		// inline const constexpr consteval inline char *const *char *const []const constexpr constantinople
		inline constexpr const char **const GENERIC_FILE         = nullptr;
		inline constexpr const char *const TEXTURE_SOURCE[]      = {".jpeg", ".jpg", ".png", ".bmp", ".dds"};
		static const size_t TEXTURE_SOURCE_LEN                   = sizeof(TEXTURE_SOURCE) / sizeof(const char *);
		inline constexpr const char *const MODEL_SOURCE[]        = {".obj", ".fbx", ".blend"};
		static const size_t MODEL_SOURCE_LEN                     = sizeof(MODEL_SOURCE) / sizeof(const char *);
		inline constexpr const char *const HEADER_SOURCE[]       = {".h", ".hpp", ".c", ".cpp", ".cc"};
		static const size_t HEADER_SOURCE_LEN                    = sizeof(HEADER_SOURCE) / sizeof(const char *);
		inline constexpr const char *const HEADER[]              = {".h", ".hpp"};
		static const size_t HEADER_LEN                           = sizeof(HEADER) / sizeof(const char *);
		inline constexpr const char *const SOURCE[]              = {".c", ".cpp", ".cc"};
		static const size_t SOURCE_LEN                           = sizeof(SOURCE) / sizeof(const char *);
		inline constexpr const char *const SHADER[]              = {".glsl"};
		static const size_t SHADER_LEN                           = sizeof(SHADER) / sizeof(const char *);
		inline constexpr const char *const FONT_SOURCE[]         = {".ttf"};
		static const size_t FONT_SOURCE_LEN                      = sizeof(FONT_SOURCE) / sizeof(const char *);
		inline constexpr const char *const VULTR_SOURCE[]        = {".vultr"};
		static const size_t VULTR_SOURCE_LEN                     = sizeof(VULTR_SOURCE) / sizeof(const char *);
		inline constexpr const char *const VULTR_ASSET_PACKAGE[] = {".vasset"};
		static const size_t VULTR_ASSET_PACKAGE_LEN              = sizeof(VULTR_ASSET_PACKAGE) / sizeof(const char *);
		inline constexpr const char *const DLL_SOURCE[]          = {".so", ".dll"};
		static const size_t DLL_SOURCE_LEN                       = sizeof(DLL_SOURCE) / sizeof(const char *);
	} // namespace FileTypes

	typedef File<FileTypes::GENERIC_FILE> GenericFile;
	typedef File<FileTypes::TEXTURE_SOURCE> TextureSource;
	typedef File<FileTypes::MODEL_SOURCE> ModelSource;
	typedef File<FileTypes::HEADER_SOURCE> HeaderAndSourceFile;
	typedef File<FileTypes::HEADER> HeaderFile;
	typedef File<FileTypes::SOURCE> SourceFile;
	typedef File<FileTypes::SHADER> ShaderSource;
	typedef File<FileTypes::FONT_SOURCE> FontSource;
	typedef File<FileTypes::VULTR_SOURCE> VultrSource;
	typedef File<FileTypes::VULTR_ASSET_PACKAGE> VultrAssetPackage;
	typedef File<FileTypes::DLL_SOURCE> DLLSource;

	// Get the name of the file, not the path.
	const char *fbasename(const IFile *file, size_t *len);

	const char *fextension(const IFile *file, bool with_dot = true);

	bool fextension_matches(const char *extension, const char *const file_type[], size_t len);

	bool fremove(const IFile *file);

	// Rename a file with the extension
	bool frename(IFile *src, const char *new_name);

	bool fmove(const IFile *src, const IFile *destination);
	bool fmove(IFile *src, const char *destination);
	bool fmove(IFile *src, const Directory *destination);

	bool fcopy(IFile *src, const char *dest);

	bool fexists(const IFile *file);

	GenericFile *dirfiles(const Directory *dir, size_t *len);

	// Casts a file (if possible) from one extension type to another
	template <const char *const extensions[]>
	bool cast_file(const GenericFile *src, File<extensions> *dest)
	{
		size_t dest_extension_count = extensions != nullptr ? sizeof(extensions) / sizeof(char *const) : 0;
		if (dest_extension_count == 0)
		{
			// dest->path = str(src->path);
			return true;
		}

		const char *extension = fextension(src);

		// If for whatever reason there is no extension on the generic file then we cannot determine if the destination is valid
		if (extension == nullptr)
		{
			return false;
		}

		for (size_t i = 0; i < dest_extension_count; i++)
		{
			const char *dest_extension = extensions[i];
			if (strcmp(dest_extension, extension) == 0)
			{
				// dest->path = str(src->path);
				return true;
			}
		}
		return false;
	}

	u16 fdate_modified(const IFile *file);

	template <const char *const extensions[]>
	bool operator<(const File<extensions> &a, const File<extensions> &b)
	{
		return strcmp(a.path, b.path) < 0;
	}

	template <const char *const extensions[]>
	bool operator==(const File<extensions> &a, const File<extensions> &b)
	{
		return strequal(a.path, b.path);
	}
} // namespace Vultr
