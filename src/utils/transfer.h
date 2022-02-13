#pragma once
#include <types/types.h>
#include <types/static_details.h>

namespace Vultr
{
	namespace Utils
	{
		inline bool is_overlapping(const void *dest, const void *src, size_t count)
		{
			if (dest >= src)
				return false;
			return dest < src && static_cast<const byte *>(dest) + count >= src;
		}

		template <typename T>
		size_t move(T *dest, const T *src, size_t count)
		{
			if constexpr (is_trivial<T>)
			{
				memmove(dest, src, count * sizeof(T));
			}
			else
			{
				for (size_t i = 0; i < count; i++)
				{
					if (dest <= src)
					{
						new (&dest[i]) T(std::move(src[i]));
					}
					else
					{
						auto index = count - i - 1;
						new (&dest[index]) T(std::move(src[index]));
					}
				}
			}
			return count;
		}

		template <typename T>
		constexpr size_t copy(T *dest, const T *src, size_t count)
		{
			size_t copied_bytes = count * sizeof(T);
			if constexpr (is_trivial<T>)
			{
				if (count == 1)
				{
					*dest = *src;
				}
				else
				{
					if (is_overlapping(dest, src, copied_bytes))
					{
						memmove(dest, src, copied_bytes);
					}
					else
					{
						memcpy(dest, src, copied_bytes);
					}
				}
				return count;
			}
			else
			{
				for (size_t i = 0; i < count; i++)
				{
					if (dest <= src)
					{
						new (&dest[i]) T(src[i]);
					}
					else
					{
						auto index = count - i - 1;
						new (&dest[index]) T(src[index]);
					}
				}
			}
		}
	} // namespace Utils
} // namespace Vultr
