#pragma once
#include "types.h"
#include <utils/traits.h>
#include "error_or.h"
#include "math/min_max.h"
#include <vultr.h>

namespace Vultr
{

	template <typename T, typename TraitsForT>
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

		struct Iterator
		{
			explicit Iterator(Bucket *bucket) : m_bucket(bucket) {}
			T &operator*() { return *m_bucket->storage(); }
			T *operator->() { return m_bucket->storage(); }

			void operator++() { skip_to_next(); }

			void skip_to_next()
			{
				if (m_bucket == nullptr)
					return;

				do
				{
					m_bucket++;
					if (m_bucket->used)
						return;
				} while (m_bucket->deleted && !m_bucket->end);

				if (m_bucket->end)
					m_bucket = nullptr;
			}

			Bucket *m_bucket = nullptr;
		};

		HashTable() = default;
		explicit HashTable(size_t capacity) : m_capacity(capacity) {}
		~HashTable() { clear(); }

		Iterator begin()
		{
			for (size_t i = 0; i < m_capacity; i++)
			{
				if (m_buckets[i].used)
				{
					return Iterator(&m_buckets[i]);
				}
			}
			return end();
		}

		Iterator end() { return Iterator(nullptr); }

		size_t size() const { return m_size; }
		size_t capacity() const { return m_capacity; }

		void clear()
		{
			if (m_buckets == nullptr)
				return;

			for (size_t i = 0; i < m_capacity; i++)
			{
				if (m_buckets[i].used)
				{
					m_buckets[i].storage()->~T();
				}
			}

			v_free(m_buckets);
			m_buckets       = nullptr;
			m_capacity      = 0;
			m_size          = 0;
			m_deleted_count = 0;
		}

		ErrorOr<void> set(const T &value)
		{
			auto *bucket = look_up_for_writing(value);
			new (bucket->storage()) T(value);
			bucket->used    = true;
			bucket->deleted = false;
		}

		ErrorOr<void> set(T &&value)
		{
			auto *bucket = look_up_for_writing(value);
			new (bucket->storage()) T(move(value));
			bucket->used    = true;
			bucket->deleted = false;
		}

		void rehash(size_t new_capacity)
		{
			new_capacity      = max<size_t>(new_capacity, 4);

			auto *old_buckets = m_buckets;
			auto old_capacity = m_capacity;

			m_buckets         = v_alloc<Bucket>(new_capacity + 1);
			m_capacity        = new_capacity;
			memset(m_buckets, 0, sizeof(Bucket) * new_capacity);

			m_buckets[m_capacity].end = true;

			for (size_t i = 0; i < old_capacity; i++)
			{
				if (old_buckets[i].used)
				{
					set(move(*old_buckets[i].storage()));
					old_buckets[i].storage()->~T();
				}
			}

			v_free(old_buckets);
		}

		const Bucket *lookup_for_reading(const T &value)
		{
			auto hash           = TraitsForT::hash(value);
			size_t bucket_index = hash % m_capacity;
			while (true)
			{
				auto &bucket = m_buckets[bucket_index];

				if (bucket.used && TraitsForT::equals(*bucket.storage(), value))
					return &bucket;

				if (!bucket.used && !bucket.deleted)
					return nullptr;

				hash         = int_hash(hash);
				bucket_index = hash % m_capacity;
			}
		}

		Bucket *lookup_for_writing(const T &value)
		{
			auto *bucket_for_reading = lookup_for_reading(value);
			if (bucket_for_reading != nullptr)
				return *const_cast<Bucket *>(bucket_for_reading);

			if ((used_bucket_count() + 1) > m_capacity)
			{
				rehash((size() + 1) * 2);
			}

			auto hash           = TraitsForT::hash(value);
			size_t bucket_index = hash % m_capacity;

			while (true)
			{
				auto &bucket = m_buckets[bucket_index];

				if (!bucket.used)
					return &bucket;

				hash         = int_hash(hash);
				bucket_index = hash % m_capacity;
			}
		}

		size_t used_bucket_count() const { return m_size + m_deleted_count; }

		Bucket *look_up_for_writing() {}

		Bucket m_buckets       = nullptr;
		size_t m_size          = 0;
		size_t m_capacity      = 0;
		size_t m_deleted_count = 0;
	};

} // namespace Vultr