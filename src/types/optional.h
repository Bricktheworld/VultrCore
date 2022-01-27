#pragma once
#include "types.h"
#include "static_details.h"

namespace Vultr
{
	struct NoneT
	{
		explicit constexpr NoneT() = default;
	};

	inline constexpr NoneT None = NoneT();

	template <typename T, bool is_reference = is_l_value<T>, bool is_pointer = is_pointer<T>>
	struct Option;

	template <typename T>
	struct Option<T, false, false>
	{
		Option() : m_has_value(false) {}
		Option(NoneT none) : m_has_value(false) {}
		Option(const T &value)
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

		template <typename U = T>
		requires(!is_same<U, bool>) operator bool() const { return has_value(); }
		// operator T() = delete;

		bool operator==(const Option &other) const
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

	template <typename ReferenceType>
	struct Option<ReferenceType, true, false>
	{
		using T  = remove_reference<ReferenceType>;
		Option() = default;
		Option(NoneT none) : m_storage(nullptr) {}
		Option(T &value) : m_storage(&value) {}
		~Option() = default;

		T &value()
		{
			ASSERT(has_value(), "Cannot return value from optional which is None!");
			return *m_storage;
		}

		const T &value() const
		{
			ASSERT(has_value(), "Cannot return value from optional which is None!");
			return *m_storage;
		}

		T &value_or(T &replacement)
		{
			if (has_value())
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
			if (has_value())
			{
				return value();
			}
			else
			{
				return replacement;
			}
		}

		void free_value() { m_storage = nullptr; }

		bool has_value() const { return m_storage != nullptr; }
		operator bool() const { return has_value(); }

		bool operator==(const Option &other) const
		{
			if (!has_value() && !other.has_value())
				return true;
			if (!has_value())
				return false;
			if (!other.has_value())
				return false;
			return &value() == &other.value();
		}

	  private:
		T *m_storage = nullptr;
	};

	template <typename PointerType>
	struct Option<PointerType, false, true>
	{
		using T  = remove_pointer<PointerType>;
		Option() = default;
		Option(NoneT none) : m_storage(nullptr) {}
		Option(T *value) : m_storage(value) { ASSERT(value != nullptr, "Cannot initialize with invalid reference!"); }
		~Option() = default;

		T *value() const
		{
			ASSERT(has_value(), "Cannot return value from optional which is None!");
			return m_storage;
		}

		T *value_or(T *replacement) const
		{
			if (has_value())
			{
				return value();
			}
			else
			{
				return replacement;
			}
		}

		void free_value() { m_storage = nullptr; }

		bool has_value() const { return m_storage != nullptr; }
		operator bool() const { return has_value(); }

		bool operator==(const Option &other) const
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
		T *m_storage = nullptr;
	};

	// NOTE(Brandon): This is really, really stupid but I kinda like it and I'm stubborn.
#define let(var, option)                                                                                                                                                                                              \
	(auto __opt = (option)) if (var = __opt.value(); false) {}                                                                                                                                                        \
	else

} // namespace Vultr
