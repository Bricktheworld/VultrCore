#pragma once
#include <tuple>
#include <string>
#include "types.h"
#include "typelist.h"

namespace Vultr
{
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
		TupleImpl() : m_value() {}
		TupleImpl(const T &value) : m_value(value) {}

		template <typename U>
		U &get()
		{
			static_assert(is_same<U, T>, "Invalid tuple access");
			return m_value;
		}

		template <typename U, size_t i>
		U &get_with_index()
		{
			static_assert(is_same<U, T> && i == 0, "Invalid tuple access");
			return m_value;
		}
	};

	template <typename T, typename... TRest>
	struct TupleImpl<T, TRest...> : TupleImpl<TRest...>
	{
		T m_value;
		TupleImpl() : m_value(), TupleImpl<TRest...>() {}
		TupleImpl(const T &first, const TRest &...rest) : m_value(first), TupleImpl<TRest...>(rest...) {}

		template <typename U>
		U &get()
		{
			if constexpr (is_same<T, U>)
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
		using Types = TypeList<Ts...>;

		Tuple() : TupleImpl<Ts...>() {}
		Tuple(const Ts &...args) : TupleImpl<Ts...>(args...) {}

		template <typename T>
		auto &get()
		{
			return TupleImpl<Ts...>::template get<T>();
		}

		template <size_t i>
		auto &get()
		{
			return TupleImpl<Ts...>::template get_with_index<typename Types::template Type<i>, i>();
		}

		template <typename ReturnT>
		ReturnT apply(ReturnT (*delegate)(Ts...))
		{
			return apply_impl(delegate, typename SequenceImpl<sizeof...(Ts)>::type());
		}

		template <typename F>
		void for_each(F &&delegate)
		{
			for_each_impl(delegate, typename SequenceImpl<sizeof...(Ts)>::type());
		}

		static constexpr size_t size() { return sizeof...(Ts); }

	  private:
		template <size_t... S, typename ReturnT>
		ReturnT apply_impl(ReturnT (*delegate)(Ts...), Sequence<S...>)
		{
			return delegate(this->get<S>()...);
		}

		template <size_t... S, typename F>
		void for_each_impl(F &&delegate, Sequence<S...>)
		{
			(delegate(S, get<S>()), ...);
		}
	};

} // namespace Vultr
