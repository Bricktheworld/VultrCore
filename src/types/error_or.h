#pragma once
#include <types/types.h>
#include "error.h"

namespace Vultr
{
	template <typename T>
	struct ErrorOr
	{
	  public:
		using ValueType = T;
		ErrorOr(const Error err) : is_error(true), error(err) {}
		ErrorOr(T value)
		{
			is_error = false;
			new (storage) T(value);
		}

		~ErrorOr()
		{
			if (!is_error)
			{
				value().~T();
				is_error = true;
			}
		}

		T &value()
		{
			ASSERT(!is_error, "Cannot return value because error was returned in ErrorOr instead! Use boolean operator on ErrorOr to make sure that it actually has a value first.");
			return *reinterpret_cast<T *>(&storage);
		}

		T &value_or(const T &replacement)
		{
			if (!is_error)
			{
				return value();
			}
			else
			{
				return replacement;
			}
		}

		Error get_error()
		{
			ASSERT(is_error, "Cannot return error because ErrorOr doesn't have an error but instead a value!");
			return error;
		}

		bool has_value() const { return !is_error; }
		operator bool() const { return has_value(); }

	  private:
		alignas(T) byte storage[sizeof(T)]{};
		Error error   = Error("Error has not been properly initialized", -1);
		bool is_error = true;
	};

	// Specialization just for void.
	template <>
	struct ErrorOr<void>
	{
	  public:
		ErrorOr() { is_error = false; }
		ErrorOr(const Error err) : is_error(true), error(err) {}
		~ErrorOr() = default;

		Error get_error()
		{
			ASSERT(is_error, "Cannot return error because ErrorOr doesn't have an error but instead a value!");
			return error;
		}

		bool has_value() const { return !is_error; }
		operator bool() const { return !is_error; }

	  private:
		Error error   = Error("Error has not been properly initialized", -1);
		bool is_error = false;
	};

#define _CONCAT(a, b) _CONCAT_INNER(a, b)
#define _CONCAT_INNER(a, b) a##b

#define _UNIQUE_NAME(base) _CONCAT(base, __COUNTER__)

#define _UNWRAP(var, err, result_var_name)                                                                                                                                                                            \
	auto result_var_name = err;                                                                                                                                                                                       \
	if (!result_var_name)                                                                                                                                                                                             \
	{                                                                                                                                                                                                                 \
		return result_var_name.get_error();                                                                                                                                                                           \
	}                                                                                                                                                                                                                 \
	var = result_var_name.value()

#define UNWRAP(var, err) _UNWRAP(var, err, _UNIQUE_NAME(__error_result))

#define VERIFY(err)                                                                                                                                                                                                   \
	{                                                                                                                                                                                                                 \
		auto __error_result = err;                                                                                                                                                                                    \
		if (!_error_result)                                                                                                                                                                                           \
			return __error_result;                                                                                                                                                                                    \
	}

// NOTE(Brandon): This is really, really stupid but I kinda like it and I'm stubborn.
#define check(err_or, var, err)                                                                                                                                                                                       \
	(auto __res = (err_or); err = __res ? __res.get_error() : Error()) if (var = (__res).value(); false) {}                                                                                                           \
	else

} // namespace Vultr