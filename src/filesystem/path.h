#pragma once
#include <types/string.h>
#include <types/error_or.h>
#include <utils/string_utils.h>

namespace Vultr
{
	struct Path
	{
		explicit Path(StringView path) : m_path(replace_all(path, "\\", "/")) { ASSERT(path.c_str() != nullptr, "Path must not be null!"); }
		~Path() = default;

		ErrorOr<bool> is_directory() const;
		ErrorOr<bool> is_file() const;

		Path operator/(const Path &other) const
		{
			ASSERT(is_directory(), "Cannot get subdirectory of path that doesn't end with '/'.");
			if (other.m_path[0] == '/')
			{
				return Path(StringView(m_path) + StringView(other.m_path));
			}
			else
			{
				return Path(StringView(m_path) + "/" + StringView(other.m_path));
			}
		}

		Path operator/(StringView other) const
		{
			ASSERT(is_directory(), "Cannot get subdirectory of path that doesn't end with '/'.");
			if (other[0] == '/')
			{
				return Path(StringView(m_path) + StringView(other));
			}
			else
			{
				return Path(StringView(m_path) + "/" + StringView(other));
			}
		}

		StringView basename()
		{
			if (m_path.length() < 2)
				return m_path;

			if (m_path.length() == 2 && m_path == "./")
			{
				return m_path;
			}

			size_t prev_slash_loc = m_path.length();
			for (size_t i = m_path.length(); i > 0; i--)
			{
				char c = m_path[i - 1];
				if (c == '/')
				{
					if (i < m_path.length())
					{
						return {m_path + i, prev_slash_loc - i};
					}
					else
					{
						prev_slash_loc = i - 1;
					}
				}
			}

			return m_path;
		}

		Option<StringView> get_extension()
		{
			auto name = this->basename();
			if let (auto i, find(name, '.'))
			{
				return StringView(name.c_str() + i, name.length() - i);
			}
			else
			{
				return None;
			}
		}

		String m_path{};
	};

	// TODO(Brandon): Implement this.
	struct PathView
	{
		PathView(const Path &path) : m_view(path.m_path) {}

		StringView m_view;
	};

	ErrorOr<void> mkdir(const Path &path);
	ErrorOr<void> mkdirp(const Path &path);

	bool exists(const Path &path);
	ErrorOr<size_t> fsize(const Path &path);
	ErrorOr<Path> pwd();
} // namespace Vultr
