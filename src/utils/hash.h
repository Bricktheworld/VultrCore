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
		const unsigned magic = 0xBA5EDB01;
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

	u32 ptr_hash(void *ptr) { return u64_hash((u64)ptr); }

	inline unsigned ptr_hash(const void *ptr) { return ptr_hash(FlatPtr(ptr)); }
} // namespace Vultr