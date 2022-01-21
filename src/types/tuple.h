#pragma once
#include <tuple>
#include <string>
#include "types.h"
#include "static_details.h"

namespace Vultr
{
	template <typename... Ts>
	struct TypeList;

	template <size_t...>
	struct Sequence
	{
	};

	template <size_t N, size_t... S>
	struct SequenceImpl : SequenceImpl<N - 1, N - 1, S...>
	{
	};

	template <size_t... S>
	struct SequenceImpl<0, S...>
	{
		typedef Sequence<S...> type;
	};

	template <typename... Ts>
	struct TupleImpl
	{
	};

	template <typename T>
	struct TupleImpl<T>
	{
		T m_value;
		TupleImpl(const T &value) : m_value(value) {}

		template <typename U>
		U &get()
		{
			static_assert(is_same<U, T>(), "Invalid tuple access");
			return m_value;
		}

		template <typename U, size_t i>
		U &get_with_index()
		{
			static_assert(is_same<U, T>() && i == 0, "Invalid tuple access");
			return m_value;
		}
	};

	template <typename T, typename... TRest>
	struct TupleImpl<T, TRest...> : TupleImpl<TRest...>
	{
		T m_value;
		TupleImpl(const T &first, const TRest &...rest) : m_value(first), TupleImpl<TRest...>(rest...) {}

		template <typename U>
		U &get()
		{
			if constexpr (is_same<T, U>())
			{
				return m_value;
			}
			else
			{
				return TupleImpl<TRest...>::template get<U>();
			}
		}

		template <typename U, size_t i>
		U &get_with_index()
		{
			if constexpr (is_same<T, U> && i == 0)
			{
				return m_value;
			}
			else
			{
				return TupleImpl<TRest...>::template get_with_index<U, i - 1>();
			}
		}
	};

	template <typename... Ts>
	struct Tuple : TupleImpl<Ts...>
	{
		template <typename T>
		auto &get()
		{
			return TupleImpl<Ts...>::template get<T>();
		}

		template <size_t i>
		auto &get()
		{
			return TupleImpl<Ts...>::template get_with_index <
		}
	};

} // namespace Vultr
