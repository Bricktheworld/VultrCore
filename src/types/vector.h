#pragma once
#include "types.h"
#include "iterator.h"
#include "utils/transfer.h"
#include <vultr.h>
#include "optional.h"

namespace Vultr
{
	/**
	 * Resizable array.
	 */
	// TODO(Brandon): At some point, if we're insane then we might consider removing the non-lvalue constraint.
	// I hate references anyway so this is fine.
	template <typename T, size_t inline_capacity = 16>
	requires(!is_l_value<T> && !is_r_value<T>) struct Vector
	{
		Vector() = default;

		explicit Vector(size_t count)
		{
			ASSERT(count != 0, "Count must be greater than 0!");
			resize(count);
		}
		Vector(const T *array, size_t count)
		{
			ASSERT(count != 0, "Count must be greater than 0!");
			ASSERT(array != nullptr, "Array must not be null!");

			resize(count);
			Utils::copy(storage(), array, count);
		}

		Vector(const Vector &other)
		{
			if (other.empty())
			{
				clear();
			}
			else
			{
				resize(other.m_size);
				Utils::copy(this->storage(), other.storage(), other.m_size);
			}
		}
		Vector(Vector &&other)
		{
			clear();
			m_size   = other.m_size;
			m_inline = other.m_inline;
			if (m_inline)
			{
				Utils::move(inline_storage(), other.inline_storage(), other.size());
			}
			else
			{
				m_storage = other.m_storage;
			}
			other.m_size    = 0;
			other.m_storage = nullptr;
			other.m_inline  = true;
		}

		Vector &operator=(const Vector &other)
		{
			if (&other == this)
			{
				return *this;
			}
			clear();
			resize(other.size());
			Utils::copy(storage(), other.storage(), other.m_size);
			return *this;
		}
		Vector &operator=(Vector &&other)
		{
			if (&other == this)
			{
				return *this;
			}
			clear();
			m_size   = other.m_size;
			m_inline = other.m_inline;
			if (m_inline)
			{
				Utils::move(inline_storage(), other.inline_storage(), other.size());
			}
			else
			{
				m_storage = other.m_storage;
			}
			other.m_size    = 0;
			other.m_storage = nullptr;
			other.m_inline  = true;
			return *this;
		}

		// Destructor for dynamic array
		~Vector() { clear(); }

		// Push element to back of dynamic_array
		// If the dynamic_array does not have enough space, then a reallocation will
		// occur
		//
		// Returns the element just inserted
		T &push_back(T &&element)
		{
			resize(m_size + 1);

			// Increase the len amount and assign the last element of array to the new
			// element
			new (&storage()[m_size - 1]) T(move(element));

			return last();
		}

		T &push_back(const T &element)
		{
			resize(m_size + 1);

			// Increase the len amount and assign the last element of array to the new
			// element
			new (&storage()[m_size - 1]) T(element);

			return last();
		}

		void push_if_not_contains(T &&element)
		{
			if (contains(element))
				return;

			push_back(forward(element));
		}

		void push_if_not_contains(const T &element)
		{
			if (contains(element))
				return;

			push_back(element);
		}

		// Inserts element at specific index in dynamic_array
		// Shifts all elements at and to the right of that index 1 to the right
		// If the dynamic_array does not have enough space, then a reallocation will
		// occur
		//
		// Throws index out of bounds error if index is greater than the
		// dynamic_array._size or if index is negative
		//
		// Returns the element just inserted
		T &insert(s32 index, T &&element)
		{
			// Fail if the index is greater than the dynamic_array._size or negative
			ASSERT(index <= m_size && index >= 0, "Index out of bounds!");

			if (index == m_size)
			{
				return push_back(forward<T>(element));
			}

			resize(m_size + 1);

			if (m_size > 1)
			{
				// Shift all elements right
				Utils::move(&storage()[index + 1], &storage()[index], m_size - index - 1);
			}

			// Assign last element of array to new element
			new (&storage()[index]) T(move(element));

			return storage()[index];
		}

		// Delete element at index in dynamic_array
		// Shifts all elements to the right of that index 1 to the left
		// If the dynamic_array len equals dynamic_array _size, then it will assume
		// nothing is reserved and a reallocation will occur unless reallocate is
		// explicitly false
		void remove(size_t index)
		{
			// Fail if the index is greater than the dynamic_array._size or negative
			ASSERT(index <= m_size && index >= 0, "Index out of bounds!");

			storage()[index].~T();

			// Shift elements to the left
			Utils::move(storage()[index], storage()[index + 1], m_size - index - 1);

			resize(m_size - 1);
		}

		// Shorthand of removing last element of the array
		void remove_last()
		{
			ASSERT(!empty(), "Array is empty!");
			remove(m_size - 1);
		}

		bool empty() const { return m_size == 0; }

		void clear()
		{
			for (size_t i = 0; i < m_size; i++)
			{
				storage()[i].~T();
			}
			if (!m_inline)
			{
				v_free(m_storage);
			}
			m_storage  = nullptr;
			m_size     = 0;
			m_capacity = inline_capacity;
			m_inline   = true;
		}

		// Ability to index into the array and assign and retrieve elements by
		// reference
		T &operator[](size_t index)
		{
			ASSERT(index < m_size && index >= 0, "Index out of bounds");
			return storage()[index];
		}
		const T &operator[](size_t index) const
		{
			ASSERT(index < m_size && index >= 0, "Index out of bounds");
			return storage()[index];
		}

		bool contains(const T &other) const
		{
			for (const auto &element : *this)
			{
				if (other == element)
					return true;
			}
			return false;
		}

		size_t size() const { return m_size; }
		size_t capacity() const { return m_capacity; }
		T &last()
		{
			ASSERT(m_size > 0, "Cannot get last from empty vector!");
			return storage()[m_size - 1];
		}

		using VIterator = Iterator<Vector, T>;

		VIterator begin() { return VIterator::begin(this); }
		VIterator end() { return VIterator::end(this); }

		using VCIterator = Iterator<const Vector, const T>;
		VCIterator begin() const { return VCIterator::begin(this); }
		VCIterator end() const { return VCIterator::end(this); }

		void resize(size_t new_size)
		{
			if (new_size == 0)
			{
				clear();
			}
			else if (m_size != new_size)
			{
				if (m_inline)
				{
					if (new_size > inline_capacity)
					{
						m_capacity = get_capacity(new_size);
						m_storage  = v_alloc<T>(m_capacity);
						Utils::move(m_storage, inline_storage(), m_size);
						m_inline = false;
					}
					m_size = new_size;
				}
				else
				{
					m_size = new_size;
					if (m_size > m_capacity || get_usage(m_size, m_capacity) < decay_threshold)
					{
						m_capacity = get_capacity(m_size);
						m_storage  = v_realloc(m_storage, m_size);
					}
				}
			}
		}

	  private:
		T *storage()
		{
			if (m_inline)
			{
				return inline_storage();
			}
			else
			{
				return m_storage;
			}
		}

		const T *storage() const
		{
			if (m_inline)
			{
				return inline_storage();
			}
			else
			{
				return m_storage;
			}
		}

		T *inline_storage() { return reinterpret_cast<T *>(m_inline_storage); }
		const T *inline_storage() const { return reinterpret_cast<const T *>(m_inline_storage); }

		T *m_storage = nullptr;
		alignas(T) byte m_inline_storage[sizeof(T) * inline_capacity]{};
		size_t m_size     = 0;
		size_t m_capacity = inline_capacity;
		bool m_inline     = true;

		static size_t get_capacity(size_t size) { return size * growth_factor; }
		static f64 get_usage(size_t size, size_t capacity) { return (f64)size / (f64)capacity; }

		// To make sure that the buffer expansion is geometric
		static constexpr f64 growth_factor   = 1.5f;
		static constexpr f64 decay_threshold = 0.3f;
	};

} // namespace Vultr
