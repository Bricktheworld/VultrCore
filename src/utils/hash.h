#pragma once
#include <types/types.h>
namespace Vultr
{
	constexpr u32 int_hash(u32 key)
	{
		key += ~(key << 15);
		key ^= (key >> 10);
		key += (key << 3);
		key ^= (key >> 6);
		key += ~(key << 11);
		key ^= (key >> 16);
		return key;
	}

	constexpr u32 double_hash(u32 key)
	{
		const u32 magic = 0xBA5EDB01;
		if (key == magic)
			return 0u;
		if (key == 0u)
			key = magic;

		key ^= key << 13;
		key ^= key >> 17;
		key ^= key << 5;
		return key;
	}

	constexpr u32 pair_int_hash(u32 key1, u32 key2) { return int_hash((int_hash(key1) * 209) ^ (int_hash(key2 * 413))); }

	constexpr u32 u64_hash(u64 key)
	{
		u32 first = key & 0xFFFFFFFF;
		u32 last  = key >> 32;
		return pair_int_hash(first, last);
	}

	inline u32 ptr_hash(void *ptr) { return u64_hash((u64)ptr); }

	constexpr u32 string_hash(str characters, size_t length, u32 seed = 0)
	{
		u32 hash = seed;
		for (size_t i = 0; i < length; ++i)
		{
			hash += (u32)characters[i];
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += hash << 3;
		hash ^= hash >> 11;
		hash += hash << 15;
		return hash;
	}

	constexpr u32 case_insensitive_string_hash(str characters, size_t length, u32 seed = 0)
	{
		// AK/CharacterTypes.h cannot be included from here.
		auto to_lowercase = [](char ch) -> u32 {
			if (ch >= 'A' && ch <= 'Z')
				return static_cast<u32>(ch) + 0x20;
			return static_cast<u32>(ch);
		};

		u32 hash = seed;
		for (size_t i = 0; i < length; ++i)
		{
			hash += to_lowercase(characters[i]);
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += hash << 3;
		hash ^= hash >> 11;
		hash += hash << 15;
		return hash;
	}
} // namespace Vultr