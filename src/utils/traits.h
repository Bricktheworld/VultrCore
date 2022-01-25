#pragma once
#include <types/types.h>
#include <types/static_details.h>
#include <utils/hash.h>

namespace Vultr
{
	template <typename T>
	struct GenericTraits
	{
		static constexpr bool is_trivial() { return false; }
		static constexpr bool equals(const T &a, const T &b) { return a == b; }
	};

	template <typename T>
	struct Traits : public GenericTraits<T>
	{
	};

	template <typename T>
	requires(is_integral<T>) struct Traits<T> : public GenericTraits<T>
	{
		static constexpr bool is_trivial() { return true; }
		static constexpr u32 hash(T value)
		{
			if constexpr (sizeof(T) < 8)
			{
				return int_hash(value);
			}
			else
			{
				return u64_hash(value);
			}
		}
	};

	template <typename T>
	requires(is_pointer<T> && !is_pointer_of_type<char, T>) struct Traits<T> : public GenericTraits<T>
	{
		static constexpr bool is_trivial() { return true; }
		static constexpr u32 hash(T value) { return ptr_hash(value); }
	};

	template <typename T>
	requires(is_pointer_of_type<char, T>) struct Traits<T> : public GenericTraits<T>
	{
		static constexpr u32 hash(const T value) { return string_hash(value, strlen(value)); }
		static constexpr bool equals(const T a, const T b) { return strcmp(a, b) == 0; }
		static constexpr bool is_trivial() { return true; }
	};

} // namespace Vultr
