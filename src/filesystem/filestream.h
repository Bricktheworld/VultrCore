#pragma once

#include "filesystem.h"

namespace Vultr
{
	enum struct StreamFormat
	{
		UTF8,
		BINARY,
	};

	enum struct StreamWriteMode
	{
		APPEND,
		OVERWRITE,
	};

	struct FileStream
	{
	  protected:
		FileStream()  = default;
		~FileStream() = default;
		static str get_format(StreamFormat format)
		{
			switch (format)
			{
				case StreamFormat::UTF8:
					return "r";
				case StreamFormat::BINARY:
					return "rb";
			}
			THROW("Something went wrong!");
		}

		static str get_write_mode(StreamFormat format, StreamWriteMode write_mode)
		{
			switch (write_mode)
			{
				case StreamWriteMode::APPEND:
					switch (format)
					{
						case StreamFormat::UTF8:
							return "a";
						case StreamFormat::BINARY:
							return "a+b";
					}
				case StreamWriteMode::OVERWRITE:
					switch (format)
					{
						case StreamFormat::UTF8:
							return "w";
						case StreamFormat::BINARY:
							return "w+b";
					}
			}
		}
	};

	template <typename T>
	concept is_valid_stream_format = is_same<T, char> || is_same<T, byte>;

	struct FileInputStream : FileStream
	{
		FileInputStream(const Path &path, StreamFormat format) : m_path(path), m_format(format)
		{
			ASSERT(path.is_file(), "Cannot open file input stream for a directory!");
			ASSERT(exists(path), "Cannot open file input stream for non-existent file!");
			m_handle = fopen(path.c_str(), get_format(format));
			ASSERT(m_handle != nullptr, "Failed to open file!");
		}

		~FileInputStream()
		{
			ASSERT(m_handle != nullptr, "Something happened with the file handle, failed to close!");
			fclose(m_handle);
		}

		template <typename T>
		requires(is_valid_stream_format<T>) ErrorOr<void> read(BufferT<T> *buf, const Option<u64> &pos = None) { return read(buf->storage, buf->size(), pos); }

		template <typename T>
		requires(is_valid_stream_format<T>) ErrorOr<void> read(T *buf, size_t size, const Option<u64> &pos = None)
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

			ASSERT(position <= size, "Buffer with size %lu does not have enough room for index %lu!", size, pos.value());
			if (fread(&buf[position], sizeof(T) * size, 1, m_handle) != 1)
			{
				return Error("Failed to read!");
			}
			return Success;
		}

		Path m_path;
		StreamFormat m_format;
		FILE *m_handle = nullptr;
	};

	struct FileOutputStream : FileStream
	{
		FileOutputStream(const Path &path, StreamFormat format, StreamWriteMode mode) : m_path(path), m_format(format), m_mode(mode)
		{
			m_handle = fopen(path.c_str(), get_write_mode(format, mode));
			ASSERT(m_handle != nullptr, "Failed to open file!");
		}

		~FileOutputStream()
		{
			ASSERT(m_handle != nullptr, "Something happened with the file handle, failed to close!");
			fclose(m_handle);
		}

		template <typename T>
		requires(is_valid_stream_format<T>) ErrorOr<void> write(const BufferT<T> &buf) { return write(buf.storage, buf.size()); }

		template <typename T>
		requires(is_valid_stream_format<T>) ErrorOr<void> write(const T *buf, size_t size)
		{
			if (fwrite(buf, sizeof(T) * size, 1, m_handle) != 1)
			{
				return Error("Failed to read!");
			}
			return Success;
		}

		Path m_path;
		StreamFormat m_format;
		StreamWriteMode m_mode;
		FILE *m_handle = nullptr;
	};

	template <typename T>
	requires(is_valid_stream_format<T>) void fread_all(const Path &path, BufferT<T> *buf) { CHECK(try_fread_all<T>(path, buf)); }

	template <typename T>
	requires(is_valid_stream_format<T>) void fread_all(const Path &path, T *buf, size_t size) { CHECK(try_fread_all<T>(path, buf, size)); }

	template <typename T>
	requires(is_valid_stream_format<T>) ErrorOr<void> try_fread_all(const Path &path, BufferT<T> *buf)
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

		TRY_UNWRAP(auto file_size, fsize(path));

		if (buf->size() < file_size)
		{
			buf->resize(file_size);
		}

		return stream.read(buf, 0);
	}

	template <typename T>
	requires(is_valid_stream_format<T>) ErrorOr<void> try_fread_all(const Path &path, T *buf, size_t size)
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

		TRY_UNWRAP(auto file_size, fsize(path));

		ASSERT(size < file_size, "Buffer is too small!");

		return stream.read(buf, size, 0);
	}

	template <typename T>
	requires(is_valid_stream_format<T>) void fwrite_all(const Path &path, const BufferT<T> &buf, StreamWriteMode write_mode) { CHECK(try_write_all<T>(path, buf, write_mode)); }

	template <typename T>
	requires(is_valid_stream_format<T>) void fwrite_all(const Path &path, const T *buf, size_t size, StreamWriteMode write_mode) { CHECK(try_write_all<T>(path, buf, size, write_mode)); }

	template <typename T>
	requires(is_valid_stream_format<T>) ErrorOr<void> try_fwrite_all(const Path &path, const BufferT<T> &buf, StreamWriteMode write_mode)
	{
		StreamFormat format;
		if constexpr (is_same<T, char>)
		{
			format = StreamFormat::UTF8;
		}
		else
		{
			format = StreamFormat::BINARY;
		}

		FileOutputStream stream(path, format, write_mode);
		TRY(stream.write(buf));
		return Success;
	}

	template <typename T>
	requires(is_valid_stream_format<T>) ErrorOr<void> try_fwrite_all(const Path &path, const T *buf, size_t size, StreamWriteMode write_mode)
	{
		StreamFormat format;
		if constexpr (is_same<T, char>)
		{
			format = StreamFormat::UTF8;
		}
		else
		{
			format = StreamFormat::BINARY;
		}

		FileOutputStream stream(path, format, write_mode);
		TRY(stream.write(buf, size));
		return Success;
	}
} // namespace Vultr