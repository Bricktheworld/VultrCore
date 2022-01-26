#pragma once
#include "types.h"
#include <utils/traits.h>
#include "error_or.h"
#include "math/min_max.h"
#include <vultr.h>

namespace Vultr
{

	template <typename T, typename TraitsForT = Traits<T>>
	struct HashTable
	{
		struct Bucket
		{
			T *storage() { return reinterpret_cast<T *>(m_storage); }
			const T *storage() const { return reinterpret_cast<T *>(m_storage); }

			bool used    = false;
			bool deleted = false;
			bool end     = false;
			alignas(T) byte m_storage[sizeof(T)]{};
		};

		struct HashTableIterator
		{
			explicit HashTableIterator(Bucket *bucket) : m_bucket(bucket) {}
			T &operator*() { return *m_bucket->storage(); }
			T *operator->() { return m_bucket->storage(); }
			bool operator==(const HashTableIterator &other) const { return m_bucket == other.m_bucket; }

			HashTableIterator &operator++()
			{
				skip_to_next();
				return *this;
			}

			HashTableIterator operator++(int)
			{
				HashTableIterator cpy = *this;
				skip_to_next();
				return cpy;
			}

			void skip_to_next()
			{
				if (m_bucket == nullptr)
					return;

				do
				{
					m_bucket++;
					if (m_bucket->used && !m_bucket->deleted)
						return;
				} while (!m_bucket->end);

				if (m_bucket->end)
					m_bucket = nullptr;
			}

			Bucket *m_bucket = nullptr;
		};

		template <typename U>
		using default_traits = conditional<is_same<U, T>, TraitsForT, Traits<U>>;

		HashTable()          = default;
		explicit HashTable(size_t capacity) : m_capacity(capacity) {}
		~HashTable() { clear(); }

		HashTableIterator begin()
		{
			for (size_t i = 0; i < m_capacity; i++)
			{
				if (m_buckets[i].used)
				{
					return HashTableIterator(&m_buckets[i]);
				}
			}
			return end();
		}

		HashTableIterator end() { return HashTableIterator(nullptr); }

		bool empty() const { return m_size == 0; }
		size_t size() const { return m_size; }
		size_t capacity() const { return m_capacity; }
		template <typename U = T, typename TraitsForU = default_traits<U>>
		bool contains(const U &value)
		{
			return find<U, TraitsForU>(value) != end();
		}

		template <typename Predicate>
		HashTableIterator find(u32 hash, Predicate predicate)
		{
			return HashTableIterator(lookup_with_hash(hash, predicate));
		}

		template <typename U, typename TraitsForU>
		static bool equals(const T &a, const U &b)
		{
			if constexpr (Equalable<TraitsForU, U, T>)
				return TraitsForU::equals(b, a);
			else if constexpr (Equalable<TraitsForU, T, U>)
				return TraitsForU::equals(a, b);
			else if constexpr (Equalable<TraitsForT, T, U>)
				return TraitsForT::equals(a, b);
			else if constexpr (Equalable<TraitsForT, U, T>)
				return TraitsForT::equals(b, a);
			return false;
		}

		template <typename U = T, typename TraitsForU = default_traits<U>>
		HashTableIterator find(const U &value)
		{
			return find(TraitsForU::hash(value), [&](T &other) { return equals<U, TraitsForU>(other, value); });
		}

		void clear()
		{
			if (m_buckets == nullptr)
				return;

			for (size_t i = 0; i < m_capacity; ++i)
			{
				if (m_buckets[i].used && !m_buckets[i].deleted)
					m_buckets[i].storage()->~T();
			}

			v_free(m_buckets);
			m_buckets       = nullptr;
			m_capacity      = 0;
			m_size          = 0;
			m_deleted_count = 0;
		}

		template <typename U = T, typename TraitsForU = default_traits<U>>
		void set(U &&value)
		{
			auto *bucket = lookup_for_writing<U, TraitsForU>(value);
			if constexpr (is_same<U, T>)
			{
				new (bucket->storage()) T(move(static_cast<T>(value)));
			}
			else
			{
				new (bucket->storage()) T(value);
			}
			bucket->used = true;
			if (bucket->deleted)
			{
				bucket->deleted = false;
				m_deleted_count--;
			}
			m_size++;
		}

		template <typename U = T, typename TraitsForU = default_traits<U>>
		void set(const U &value)
		{
			auto *bucket = lookup_for_writing<U, TraitsForU>(value);
			if constexpr (is_same<U, T>)
			{
				new (bucket->storage()) T(move(static_cast<T>(value)));
			}
			else
			{
				new (bucket->storage()) T(value);
			}
			bucket->used = true;
			if (bucket->deleted)
			{
				bucket->deleted = false;
				m_deleted_count--;
			}
			m_size++;
		}

		template <typename U = T, typename TraitsForU = default_traits<U>>
		bool remove(const U &value)
		{
			auto it = find<U, TraitsForU>(value);
			if (it != end())
			{
				remove(it);
				return true;
			}
			return false;
		}

		template <typename Predicate>
		size_t remove_all_matching(Predicate predicate)
		{
			size_t removed_count = 0;
			for (auto it = begin(); it != end();)
			{
				if (predicate(*it))
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

		HashTableIterator remove(const HashTableIterator &iterator)
		{
			auto *bucket = iterator.m_bucket;
			ASSERT(bucket->used && !bucket->deleted, "Cannot remove bucket that is not used or deleted!");

			auto next_iterator = iterator;
			next_iterator++;

			bucket->storage()->~T();
			bucket->used    = false;
			bucket->deleted = true;
			m_size--;
			m_deleted_count++;
			return next_iterator;
		}

		void rehash(size_t new_capacity)
		{
			new_capacity      = max<size_t>(new_capacity, 4);

			auto *old_buckets = m_buckets;
			auto old_capacity = m_capacity;

			m_buckets         = v_alloc<Bucket>(new_capacity + 1);
			m_capacity        = new_capacity;
			for (size_t i = 0; i < new_capacity + 1; i++)
			{
				new (&m_buckets[i]) Bucket();
			}

			m_buckets[m_capacity].end = true;

			if (old_buckets == nullptr)
				return;

			for (size_t i = 0; i < old_capacity; i++)
			{
				if (old_buckets[i].used)
				{
					auto *storage = old_buckets[i].storage();
					auto *bucket  = lookup_for_writing(*storage);
					new (bucket->storage()) T(move(*storage));
					bucket->used = true;

					old_buckets[i].storage()->~T();
				}
			}

			v_free(old_buckets);
		}

		template <typename U = T, typename TraitsForU = default_traits<U>>
		Bucket *lookup_for_writing(const U &value)
		{
			if ((used_bucket_count() + 1) >= m_capacity)
			{
				rehash((size() + 1) * 2);
			}

			auto hash                  = TraitsForU::hash(value);
			Bucket *first_empty_bucket = nullptr;

			while (true)
			{
				auto *bucket = &m_buckets[hash % m_capacity];

				if (bucket->used && equals<U, TraitsForU>(*bucket->storage(), value))
					return bucket;

				if (!bucket->used)
				{
					if (first_empty_bucket == nullptr)
						first_empty_bucket = bucket;

					if (!bucket->deleted)
						return first_empty_bucket;
				}

				hash = double_hash(hash);
			}
		}

		template <typename Predicate>
		Bucket *lookup_with_hash(u32 hash, Predicate predicate)
		{
			if (empty())
				return nullptr;

			while (true)
			{
				auto *bucket = &m_buckets[hash % m_capacity];

				if (bucket->used && predicate(*bucket->storage()))
					return bucket;

				if (!bucket->used && !bucket->deleted)
					return nullptr;

				hash = double_hash(hash);
			}
		}

		size_t used_bucket_count() const { return m_size + m_deleted_count; }

		Bucket *m_buckets      = nullptr;
		size_t m_size          = 0;
		size_t m_capacity      = 0;
		size_t m_deleted_count = 0;
	};

} // namespace Vultr