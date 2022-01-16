#pragma once
#include <types/types.h>
#include "iterator.h"

namespace Vultr
{
	template <typename T, size_t length>
	struct Array
	{
		Array()  = default;
		~Array() = default;
		T &operator[](size_t index)
		{
			ASSERT(index < size() && index >= 0, "Index out of bounds!");
			return m_storage[index];
		}

		const T &operator[](size_t index) const
		{
			ASSERT(index < size() && index >= 0, "Index out of bounds!");
			return m_storage[index];
		}

		size_t size() const { return length; }
		using AConstIterator = Iterator<Array const, T const>;
		using AIterator      = Iterator<Array, T>;

		AConstIterator begin() const { return AConstIterator::begin(this); }
		AIterator begin() { return AIterator::begin(this); }

		AConstIterator end() const { return AConstIterator::end(this); }
		AIterator end() { return AIterator::end(this); }

	  private:
		T m_storage[length];
	};
} // namespace Vultr
