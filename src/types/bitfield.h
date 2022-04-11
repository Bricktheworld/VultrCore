#pragma once
#include <types/types.h>
#include <math/min_max.h>
#include <bitset>

namespace Vultr
{
	template <size_t bits>
	struct Bitfield
	{
	  public:
		Bitfield() = default;
		Bitfield(u64 data) : m_elements({data}){};
		~Bitfield() = default;

		void set(size_t index, bool val)
		{
			ASSERT(index < bits, "Index is greater than the number of bits available.");
			size_t element_index = index / 64;
			size_t shifted       = 1 << index;
			if (!val)
			{
				shifted = ~shifted;
				m_elements[element_index] &= shifted;
			}
			else
			{
				m_elements[element_index] |= shifted;
			}
		}

		bool at(size_t index) const
		{
			ASSERT(index < bits, "Index is greater than the number of bits available.");
			size_t element_index = index / 64;
			size_t shifted       = 1 << (index % 64);
			return m_elements[element_index] & shifted;
		}

		bool operator[](size_t index) const { return at(index); }

		template <size_t other_bits>
		Bitfield &operator&=(const Bitfield<other_bits> &other)
		{
			constexpr size_t elements = min(m_num_elements, Bitfield<other_bits>::m_num_elements);
			for (size_t i = 0; i < elements; i++)
				m_elements[i] &= other.m_elements[i];
			return *this;
		}

		template <size_t other_bits>
		auto operator&(const Bitfield<other_bits> &other) const
		{
			Bitfield<max(bits, other_bits)> res{};
			for (size_t i = 0; i < min(m_num_elements, other.m_num_elements); i++)
			{
				res.m_elements[i] = m_elements[i] & other.m_elements[i];
			}
			return res;
		}

		template <size_t other_bits>
		Bitfield &operator|=(const Bitfield<other_bits> &other)
		{
			constexpr size_t elements = min(m_num_elements, Bitfield<other_bits>::m_num_elements);
			for (size_t i = 0; i < elements; i++)
				m_elements[i] |= other.m_elements[i];
			return *this;
		}

		template <size_t other_bits>
		auto operator|(const Bitfield<other_bits> &other) const
		{
			Bitfield<max(bits, other_bits)> res;
			for (size_t i = 0; i < max(m_num_elements, other.m_num_elements); i++)
			{
				if (i > m_num_elements)
				{
					res.m_elements[i] = other.m_elements[i];
				}
				else if (i > other.m_num_elements)
				{
					res.m_elements[i] = m_elements[i];
				}
				else
				{
					res.m_elements[i] = m_elements[i] | other.m_elements[i];
				}
			}
			return res;
		}

		template <size_t other_bits>
		Bitfield &operator^=(const Bitfield<other_bits> &other)
		{
			constexpr size_t elements = min(m_num_elements, Bitfield<other_bits>::m_num_elements);
			for (size_t i = 0; i < elements; i++)
				m_elements[i] ^= other.m_elements[i];
			return *this;
		}

		template <size_t other_bits>
		auto operator^(const Bitfield<other_bits> &other) const
		{
			Bitfield<max(bits, other_bits)> res;
			for (size_t i = 0; i < max(m_num_elements, other.m_num_elements); i++)
			{
				if (i > m_num_elements)
				{
					res.m_elements[i] = other.m_elements[i] ^ 0;
				}
				else if (i > other.m_num_elements)
				{
					res.m_elements[i] = m_elements[i] ^ 0;
				}
				else
				{
					res.m_elements[i] = m_elements[i] ^ other.m_elements[i];
				}
			}
			return res;
		}

		bool operator==(const Bitfield &other) const
		{
			for (size_t i = 0; i < m_num_elements; i++)
			{
				if (m_elements[i] != other.m_elements[i])
					return false;
			}
			return true;
		}

		auto operator~() const
		{
			Bitfield<bits> res;
			for (size_t i = 0; i < m_num_elements; i++)
				res.m_elements[i] = ~m_elements[i];
			return res;
		}

		constexpr static size_t m_num_elements = max<size_t>(bits / 64, 1);
		u64 m_elements[m_num_elements]{};
	};
} // namespace Vultr
