#pragma once

#include "path.h"
#define _FILE_OFFSET_BITS 64

namespace Vultr
{
	enum struct StreamFormat
	{
		UTF8,
		BINARY,
	};

	enum struct StreamWriteFormat
	{
		APPEND,
		OVERWRITE,
	};

	template <typename T>
	concept is_valid_stream_format = is_same<T, char> || is_same<T, byte>;

	struct FileInputStream
	{
		FileInputStream(const Path &path, StreamFormat format) : m_path(path), m_format(format)
		{
			ASSERT(path.is_file(), "Cannot open file input stream for a directory!");
			ASSERT(exists(path), "Cannot open file input stream for non-existent file!");
			m_handle = fopen(path.m_path.c_str(), get_mode());
			ASSERT(m_handle != nullptr, "Failed to open file!");
		}

		~FileInputStream()
		{
			ASSERT(m_handle != nullptr, "Something happened with the file handle, failed to close!");
			fclose(m_handle);
		}

		template <typename T>
		requires(is_valid_stream_format<T>) ErrorOr<void> read(BufferT<T> *buf, const Option<u64> pos = None)
		{
			u64 position;
			if (pos.has_value())
			{
				position = pos.value();
			}
			else
			{
				position = ftello(m_handle);
			}

			ASSERT(position <= buf->size(), "Buffer with size %s does not have enough room for index %lu!", buf->size(), pos);
			if (fread(&buf->storage[position], buf->size(), 1, m_handle) != 1)
			{
				return Error("Failed to read!");
			}
		}

	  private : str get_mode()
		{
			switch (m_format)
			{
				case StreamFormat::UTF8:
					return "r";
				case StreamFormat::BINARY:
					return "rb";
			}
		}

		Path m_path;
		StreamFormat m_format;
		FILE *m_handle = nullptr;
	};

	template <typename T>
	requires(is_valid_stream_format<T>) ErrorOr<void> fread_all(const Path &path, BufferT<T> *buf)
	{
		ASSERT(exists(path), "File does not exist!");
		ASSERT(path.is_file(), "Cannot read from a directory!");
		StreamFormat format;
		if constexpr (is_same<T, char>)
		{
			format = StreamFormat::UTF8;
		}
		else
		{
			format = StreamFormat::BINARY;
		}

		FileInputStream stream(path, format);

		UNWRAP(auto file_size, fsize(path));

		if (buf->size() < file_size)
		{
			buf->resize(file_size);
		}

		return stream.read(buf, 0);
	}
} // namespace Vultr