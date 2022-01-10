#pragma once
#include <types/types.h>

namespace Vultr
{
	struct NoneT
	{
		explicit constexpr NoneT() = default;
	};

	inline constexpr NoneT None = NoneT();

	template <typename T>
	struct Option
	{
	  public:
		Option() : m_has_value(false) {}
		Option(NoneT none) : m_has_value(false) {}
		Option(T value)
		{
			m_has_value = true;
			new (m_storage) T(value);
		}
		~Option()
		{
			if (m_has_value)
			{
				free_value();
			}
		}

		T &value()
		{
			ASSERT(m_has_value, "Cannot return value from optional which is None!");
			return *reinterpret_cast<T *>(&m_storage);
		}

		const T &value() const
		{
			ASSERT(m_has_value, "Cannot return value from optional which is None!");
			return *reinterpret_cast<const T *>(&m_storage);
		}

		T &value_or(const T &replacement)
		{
			if (m_has_value)
			{
				return value();
			}
			else
			{
				return replacement;
			}
		}

		const T &value_or(const T &replacement) const
		{
			if (m_has_value)
			{
				return value();
			}
			else
			{
				return replacement;
			}
		}

		void free_value()
		{
			if (m_has_value)
			{
				value().~T();
				m_has_value = false;
			}
		}

		bool has_value() const { return m_has_value; }
		operator bool() const { return has_value(); }
		// operator T() = delete;

		bool operator==(const Option<T> &other) const
		{
			if (!has_value() && !other.has_value())
				return true;
			if (!has_value())
				return false;
			if (!other.has_value())
				return false;
			return value() == other.value();
		}

	  private:
		alignas(T) byte m_storage[sizeof(T)]{};
		bool m_has_value = false;
	};

	// NOTE(Brandon): This is really, really stupid but I kinda like it and I'm stubborn.
#define let(var, option)                                                                                                                                                                                              \
	(auto __opt = (option)) if (var = __opt.value(); false) {}                                                                                                                                                        \
	else

} // namespace Vultr
