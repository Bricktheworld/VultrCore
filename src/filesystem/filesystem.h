#pragma once
#include <types/string.h>
#include <types/error_or.h>
#include <utils/string_utils.h>
#include <platform/platform.h>

namespace Vultr
{
	struct Path
	{
		Path() = default;
		explicit Path(StringView path) : m_path(replace_all(path, "\\", "/")) { ASSERT(path.c_str() != nullptr, "Path must not be null!"); }
		~Path() = default;

		bool is_directory() const;
		bool is_file() const;

		Path operator/(const Path &other) const { return *this / other.string(); }

		Path operator/(StringView other) const
		{
			ASSERT(is_directory(), "Cannot get subdirectory of path that isn't a directory!");
			auto trailing_slash = String(m_path.last() == '/' ? "" : "/");
			if (other[0] == '/')
			{
				return Path(string() + trailing_slash + other.substr(1));
			}
			else
			{
				return Path(string() + trailing_slash + other);
			}
		}

		bool operator==(const Path &other) const { return string() == other.string(); }

		StringView basename() const
		{
			if (m_path.length() < 2)
				return m_path;

			if (m_path.length() == 2 && m_path == "./")
			{
				return m_path;
			}

			size_t prev_slash_loc = m_path.length();
			ASSERT(m_path.length() > 0, "Cannot get base name for empty path!");
			for (size_t i = m_path.length(); i > 0; i--)
			{
				char c = m_path[i - 1];
				if (c == '/')
				{
					if (i < m_path.length())
					{
						return {m_path.c_str() + i, prev_slash_loc - i};
					}
					else
					{
						prev_slash_loc = i - 1;
					}
				}
			}

			return m_path;
		}

		Option<StringView> get_extension() const
		{
			auto name = this->basename();
			if let (auto i, find_last(name, '.'))
			{
				return StringView(name.c_str() + i, name.length() - i);
			}
			else
			{
				return None;
			}
		}

		StringView string() const { return m_path.c_str(); }
		str c_str() const { return string().c_str(); }

	  private:
		String m_path{};
	};

	ErrorOr<void> makedir(const Path &path);
	ErrorOr<void> makedirp(const Path &path);

	bool exists(const Path &path);
	ErrorOr<size_t> fsize(const Path &path);
	ErrorOr<u64> fget_date_modified_ms(const Path &path);
	ErrorOr<Path> pwd();
	bool has_parent(const Path &path);
	ErrorOr<Path> get_parent(const Path &path);
	ErrorOr<void> ftouch(const Path &path);

	struct DirectoryIterator
	{
		explicit DirectoryIterator(const Path &path) : path(path)
		{
			ASSERT(exists(path), "Cannot iterate through directory that does not exist!");
			ASSERT(path.is_directory(), "Cannot iterate through path that is not a directory!");

			CHECK_UNWRAP(dir_handle, Platform::Filesystem::open_dir(path.c_str()));
		}
		~DirectoryIterator() { Platform::Filesystem::close_dir(dir_handle); }

		struct Iterator
		{
			Iterator(const Path &path, Platform::Filesystem::DirectoryHandle *dir_handle, const Option<Platform::Filesystem::DirectoryEntry> &entry) : base_dir(path), dir_handle(dir_handle), entry(entry) {}

			bool operator==(Iterator other) const
			{
				if (!entry.has_value() && !other.entry.has_value())
					return true;
				if (!entry.has_value())
					return false;
				if (!other.entry.has_value())
					return false;
				return entry.value().uuid == other.entry.value().uuid;
			}

			auto operator*() const { return base_dir / Path(entry.value().name); }
			auto operator->() const { return base_dir / Path(entry.value().name); }

			Iterator operator++()
			{
				next();
				return *this;
			}

		  private:
			void next()
			{
				if check (Platform::Filesystem::read_dir(dir_handle), auto ent, auto _)
				{
					entry = ent;
				}
				else
				{
					entry = None;
				}
			}

			Path base_dir{};
			Option<Platform::Filesystem::DirectoryEntry> entry = None;
			Platform::Filesystem::DirectoryHandle *dir_handle  = nullptr;
		};

		Iterator begin()
		{
			if check (Platform::Filesystem::read_dir(dir_handle), auto entry, auto _)
			{
				return {path, dir_handle, entry};
			}
			else
			{
				return {path, dir_handle, None};
			}
		}
		Iterator end() { return {path, dir_handle, None}; }

	  private:
		Path path{};
		Platform::Filesystem::DirectoryHandle *dir_handle = nullptr;
	};

	template <>
	struct Traits<Path> : public GenericTraits<Path>
	{
		static u32 hash(const Path &p) { return p.string().hash(); }
		static u32 equals(const Path &a, const Path &b) { return a == b; }
		static u32 equals(const Path &a, str b) { return a.string() == b; }
	};
} // namespace Vultr
