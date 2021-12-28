#pragma once
#include <tuple>
#include <string>
#include "types.h"

namespace Vultr
{

	template <size_t i, typename T>
	struct TupleLeaf
	{
		T val;
		TupleLeaf(const T &val) : val(val) {}
	};

	template <size_t i, typename... Ts>
	struct TupleImpl;

	template <size_t i>
	struct TupleImpl<i>
	{
	};

	template <size_t i, typename CurrT, typename... TailTs>
	struct TupleImpl<i, CurrT, TailTs...> : public TupleLeaf<i, CurrT>, public TupleImpl<i + 1, TailTs...>
	{
		TupleImpl(const CurrT &curr, const TailTs &...rest) : TupleLeaf<i, CurrT>(curr), TupleImpl<i + 1, TailTs...>(rest...) {}
	};

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

	/**
	 * THIS IS PRONOUNCED TWO-PLE!!!!!!!!!!!
	 * NO TUHPLE's ALLOWED
	 */
	template <typename... Ts>
	struct Tuple : TupleImpl<0, Ts...>
	{
		Tuple() = default;
		Tuple(const Ts &...args) : TupleImpl<0, Ts...>(args...) {}

		template <size_t i, typename T>
		T &get()
		{
			return TupleLeaf<i, T>::val;
		}

		template <typename ReturnT>
		ReturnT apply(ReturnT (*delegate)(Ts...))
		{
			return apply_impl(delegate, typename SequenceImpl<sizeof...(Ts)>::type());
		}

	  private:
		template <size_t... S, typename ReturnT>
		ReturnT apply_impl(ReturnT (*delegate)(Ts...), Sequence<S...>)
		{
			return delegate(this->get<S, Ts>()...);
		}
	};
} // namespace Vultr
