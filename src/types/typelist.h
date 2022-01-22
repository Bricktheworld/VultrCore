#pragma once
#include "types.h"
#include "static_details.h"

namespace Vultr
{
	template <typename... Ts>
	struct TypeList;

	template <size_t index, typename List>
	struct TypeListElement;

	template <size_t index, typename Head, typename... Tail>
	struct TypeListElement<index, TypeList<Head, Tail...>> : TypeListElement<index - 1, TypeList<Tail...>>
	{
	};

	template <typename Head, typename... Tail>
	struct TypeListElement<0, TypeList<Head, Tail...>>
	{
		typedef Head Type;
	};

	template <typename... Types>
	struct TypeList
	{
		static constexpr size_t size = sizeof...(Types);

		template <size_t i>
		using Type = typename TypeListElement<i, TypeList<Types...>>::Type;

		template <typename T>
		constexpr size_t index_of()
		{
			static_assert(Contains<T, Types...>::contains, "Type list does not contain type T!");
			return IndexOf<T, Types...>::index;
		}

		template <typename T>
		constexpr bool contains()
		{
			return Contains<T, Types...>::contains;
		}
	};
} // namespace Vultr
