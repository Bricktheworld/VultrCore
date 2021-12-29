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
		Option() : has_value(false) {}
		Option(NoneT none) : has_value(false) {}
		Option(T value)
		{
			has_value = true;
			new (storage) T(value);
		}
		~Option()
		{
			if (has_value)
			{
				free_value();
			}
		}

		T &value()
		{
			ASSERT(has_value, "Cannot return value from optional which is None!");
			return *reinterpret_cast<T *>(&storage);
		}

		T &value_or(const T &replacement)
		{
			if (has_value)
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
			if (has_value)
			{
				value().~T();
				has_value = false;
			}
		}

		operator bool() const { return has_value; }

	  private:
		alignas(T) byte storage[sizeof(T)]{};
		bool has_value = false;
	};

	// NOTE(Brandon): This is really, really stupid but I kinda like it and I'm stubborn.
#define let(var, option)                                                                                                                                                                                              \
	(auto __opt = (option)) if (var = __opt.value(); false) {}                                                                                                                                                        \
	else

} // namespace Vultr