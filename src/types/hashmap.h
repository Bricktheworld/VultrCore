#pragma once
#include <utils/traits.h>
#include "hashtable.h"

namespace Vultr
{
	template <typename K, typename V, typename TraitsForK = Traits<K>>
	requires(!is_l_value<V> && !is_r_value<V> && !is_l_value<K> && !is_r_value<K>) struct Hashmap
	{
		struct Entry
		{
			const K key;
			V value;
		};

		template <typename U, typename TraitsForU>
		requires(requires(const U &key) { TraitsForU::hash(key); }) struct EntryTraits
		{
			static u32 hash(const Entry &entry) { return TraitsForK::hash(entry.key); }
			static u32 hash(const U &key) { return TraitsForU::hash(key); }
			static bool equals(const Entry &a, const Entry &b) { return TraitsForK::equals(a.key, b.key); }
			static bool equals(const Entry &a, const U &b) { return HashTable<K, TraitsForK>::template equals<U, TraitsForU>(a.key, b); }
		};

		template <typename U>
		using default_traits = conditional<is_same<U, K>, TraitsForK, Traits<U>>;

		using Traits         = EntryTraits<K, TraitsForK>;
		using HashTableType  = HashTable<Entry, Traits>;
		using HIterator      = typename HashTableType::HashTableIterator;

		Hashmap()            = default;

		HIterator begin() { return m_table.begin(); }
		HIterator end() { return m_table.end(); }

		template <typename Predicate>
		HIterator find(u32 hash, Predicate predicate)
		{
			return m_table.find(hash, predicate);
		}

		template <typename U = K, typename TraitsForU = default_traits<U>>
		HIterator find(const U &value)
		{
			return m_table.template find<U, EntryTraits<U, TraitsForU>>(value);
		}

		bool empty() const { return m_table.empty(); }
		size_t size() const { return m_table.size(); }
		size_t capacity() const { return m_table.capacity(); }
		void clear() { m_table.clear(); }

		template <typename U = K, typename TraitsForU = default_traits<U>>
		bool contains(const U &key)
		{
			return m_table.template contains<U, EntryTraits<U, TraitsForU>>(key);
		}

		template <typename U = K>
		void set(const U &key, const V &value)
		{
			if constexpr (is_same<U, K>)
			{
				return m_table.set({static_cast<K>(key), move(value)});
			}
			else
			{
				return m_table.set({K(key), move(value)});
			}
		}
		template <typename U = K>
		void set(const U &key, V &&value)
		{
			if constexpr (is_same<U, K>)
			{
				return m_table.set({static_cast<K>(key), move(value)});
			}
			else
			{
				return m_table.set({K(key), move(value)});
			}
		}
		template <typename U = K>
		void set(U &&key, V &&value)
		{
			if constexpr (is_same<U, K>)
			{
				return m_table.set({move(static_cast<K>(key)), move(value)});
			}
			else
			{
				return m_table.set({K(key), move(value)});
			}
		}

		template <typename U = K, typename TraitsForU = default_traits<U>>
		bool remove(const U &value)
		{
			return m_table.template remove<U, EntryTraits<U, TraitsForU>>(value);
		}

		template <typename Predicate>
		size_t remove_all_matching(Predicate predicate)
		{
			size_t removed_count = 0;
			for (auto it = begin(); it != end();)
			{
				if (predicate(it->key, it->value))
				{
					it = remove(it);
					removed_count++;
				}
				else
				{
					it++;
				}
			}
			return removed_count;
		}

		HIterator remove(const HIterator &iterator) { return m_table.remove(iterator); }

		template <typename U = K, typename TraitsForU = default_traits<U>>
		V &get(const U &key)
		{
			if let (auto value, try_get(key))
				return value;
			else
				THROW("Not found!");
		}

		template <typename U = K, typename TraitsForU = default_traits<U>>
		Option<V &> try_get(const U &value)
		{
			HIterator it = find<U, TraitsForU>(value);

			if (it == end())
				return None;

			return it->value;
		}

		HashTableType m_table;
	};
} // namespace Vultr