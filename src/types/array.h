#pragma once
#include <types/types.h>
#include "iterator.h"

namespace Vultr
{
	template <typename T, size_t length>
	struct Array
	{
		constexpr Array() = default;
		constexpr Array(const T (&array)[length]) { Utils::copy(m_storage, array, length); }
		~Array() = default;

		constexpr T &operator[](size_t index)
		{
			ASSERT(index < size() && index >= 0, "Index out of bounds!");
			return m_storage[index];
		}

		constexpr const T &operator[](size_t index) const
		{
			ASSERT(index < size() && index >= 0, "Index out of bounds!");
			return m_storage[index];
		}

		constexpr size_t size() const { return length; }
		using AConstIterator = Iterator<Array const, T const>;
		using AIterator      = Iterator<Array, T>;

		constexpr AConstIterator begin() const { return AConstIterator::begin(this); }
		constexpr AIterator begin() { return AIterator::begin(this); }

		constexpr AConstIterator end() const { return AConstIterator::end(this); }
		constexpr AIterator end() { return AIterator::end(this); }

	  private:
		T m_storage[length]{};
	};
} // namespace Vultr
