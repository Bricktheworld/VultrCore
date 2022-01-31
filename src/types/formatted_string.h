#pragma once
#include "types.h"
#include "iterator.h"
#include "string_view.h"

namespace Vultr
{
	template <typename T, size_t Size>
	struct Array
	{
		constexpr static size_t size() { return Size; }
		constexpr const T &operator[](size_t index) const { return m_storage[index]; }
		constexpr T &operator[](size_t index) { return m_storage[index]; }

		using AIterator = Iterator<Array, T>;

		constexpr AIterator begin() const { return AIterator::begin(this); }
		constexpr AIterator end() { return AIterator::end(this); }

		T m_storage[Size];
	};

	template <typename... Args>
	constexpr void compiletime_fail(Args...);

	template <typename... Args>
	struct FormattedString
	{
		template <size_t n>
		consteval FormattedString(const char (&format)[n], Args...) : m_string(format, n)
		{
			check_format_parameter_consistency<n, sizeof...(Args)>(format);
		}

		template <size_t n, size_t param_count>
		constexpr static void check_format_parameter_consistency(const char (&fmt)[n])
		{
			if constexpr (param_count == 0)
			{
				for (size_t i = 0; i < n; i++)
				{
					auto c = fmt[i];
					switch (c)
					{
						case '{':
						case '}':
							if (i > 0 && fmt[i - 1] == '\\')
								continue;

							compiletime_fail("No parameters specified for formatted string!");
							break;
					}
				}
			}
			else
			{
				Array<size_t, param_count> format_position_indices{};
				size_t format_index = 0;
				for (size_t i = 0; i < n; i++)
				{
					auto c = fmt[i];
					switch (c)
					{
						case '{':
							if (i + 1 < n)
							{
								if (fmt[i + 1] == '}')
								{
									format_position_indices[format_index] = i;
									i++;
									format_index++;
									continue;
								}
								else if (fmt[i + 1] == '{')
								{
									i++;
									continue;
								}
							}
							compiletime_fail("Unpaired '{' found!");
							break;
						case '}':
							if (i + 1 < n && fmt[i + 1] == '}')
							{
								i++;
								continue;
							}
							compiletime_fail("Unpaired '}' found!");
							break;
					}
				}
				if (format_index < param_count - 1)
				{
					compiletime_fail("Not all provided arguments are used in format string!");
				}
			}
		}

		StringView m_string;
	};
} // namespace Vultr