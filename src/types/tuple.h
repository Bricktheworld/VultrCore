#pragma once
#include <tuple>
#include <string>
#include "types.h"

namespace vtl
{
    // template <class Fn, typename Arg>
    // decltype(auto) apply_if_invocable(Fn fn, Arg &&arg)
    // {
    //     if constexpr (std::is_invocable_v<Fn, Arg>)
    //         return fn(std::forward<Arg>(arg));
    //     else
    //         return std::forward<Arg>(arg);
    // }

    // template <typename Tuple, class Fn, size_t... Is>
    // auto apply_if_impl(Tuple &&t, Fn fn, std::index_sequence<Is...>)
    // {
    //     return std::make_tuple(apply_if_invocable(fn, std::get<Is>(std::forward<Tuple>(t)))...);
    // }

    // template <class Tuple, class Fn>
    // auto apply_if(Tuple &&t, Fn fn)
    // {
    //     return apply_if_impl(std::forward<Tuple>(t), fn, std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
    // }

    // template <bool>
    // class Select
    // {
    //   public:
    //     template <typename F, typename T>
    //     T &operator()(F &, T &t) const
    //     {
    //         return t;
    //     }
    // };

    // template <>
    // class Select<true>
    // {
    //   public:
    //     template <typename F, typename T>
    //     auto operator()(F &f, T &t) const -> decltype(f(t))
    //     {
    //         return f(t);
    //     }
    // };

    // template <typename Fn, typename Tuple, size_t... Is>
    // auto apply_if_impl(Tuple t, Fn &&f, std::index_sequence<Is...>)
    // {
    //     return std::make_tuple(Select<std::is_same_v<std::string, std::tuple_element_t<Is, Tuple>>>()(f, std::get<Is>(t))...);
    // }
} // namespace vtl
