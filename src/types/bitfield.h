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
		constexpr Bitfield() = default;
		constexpr Bitfield(u32 initialized_bit) { this->set(initialized_bit, true); };
		//		constexpr Bitfield(u64 data[bits]) {m_elements = data;}
		~Bitfield() = default;

		constexpr auto &set(size_t index, bool val)
		{
			ASSERT(index < bits, "Index is greater than the number of bits available.");
			size_t element_index = index / 64;
			size_t shifted       = 1UL << (index % 64);
			if (!val)
			{
				shifted = ~shifted;
				m_elements[element_index] &= shifted;
			}
			else
			{
				m_elements[element_index] |= shifted;
			}
			return *this;
		}

		constexpr bool at(size_t index) const
		{
			ASSERT(index < bits, "Index is greater than the number of bits available.");
			size_t element_index = index / 64;
			size_t shifted       = 1 << (index % 64);
			return m_elements[element_index] & shifted;
		}

		constexpr bool operator[](size_t index) const { return at(index); }

		template <size_t other_bits>
		constexpr Bitfield &operator&=(const Bitfield<other_bits> &other)
		{
			constexpr size_t elements = min(m_num_elements, Bitfield<other_bits>::m_num_elements);
			for (size_t i = 0; i < elements; i++)
				m_elements[i] &= other.m_elements[i];
			return *this;
		}

		template <size_t other_bits>
		constexpr auto operator&(const Bitfield<other_bits> &other) const
		{
			Bitfield<max(bits, other_bits)> res{};
			for (size_t i = 0; i < min(m_num_elements, other.m_num_elements); i++)
			{
				res.m_elements[i] = m_elements[i] & other.m_elements[i];
			}
			return res;
		}

		template <size_t other_bits>
		constexpr Bitfield &operator|=(const Bitfield<other_bits> &other)
		{
			constexpr size_t elements = min(m_num_elements, Bitfield<other_bits>::m_num_elements);
			for (size_t i = 0; i < elements; i++)
				m_elements[i] |= other.m_elements[i];
			return *this;
		}

		template <size_t other_bits>
		constexpr auto operator|(const Bitfield<other_bits> &other) const
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
		constexpr Bitfield &operator^=(const Bitfield<other_bits> &other)
		{
			constexpr size_t elements = min(m_num_elements, Bitfield<other_bits>::m_num_elements);
			for (size_t i = 0; i < elements; i++)
				m_elements[i] ^= other.m_elements[i];
			return *this;
		}

		template <size_t other_bits>
		constexpr auto operator^(const Bitfield<other_bits> &other) const
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

		constexpr bool operator==(const Bitfield &other) const
		{
			for (size_t i = 0; i < m_num_elements; i++)
			{
				if (m_elements[i] != other.m_elements[i])
					return false;
			}
			return true;
		}

		constexpr auto operator~() const
		{
			Bitfield<bits> res;
			for (size_t i = 0; i < m_num_elements; i++)
				res.m_elements[i] = ~m_elements[i];
			return res;
		}

		constexpr auto &set_all()
		{
			for (auto &element : m_elements)
				element = U64Max;
			return *this;
		}

		constexpr auto &clear()
		{
			for (auto &element : m_elements)
				element = 0;
			return *this;
		}

		constexpr static size_t m_num_elements = max<size_t>(bits / 64, 1);
		u64 m_elements[m_num_elements]{};
	};

	template <size_t bits>
	static constexpr Bitfield<bits> MAX_BITFIELD = Bitfield<bits>().set_all();
} // namespace Vultr
