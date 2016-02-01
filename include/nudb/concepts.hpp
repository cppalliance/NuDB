//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_CONCEPTS_HPP
#define NUDB_CONCEPTS_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace nudb {

namespace detail {

template<class T>
class is_Hasher
{
    template<class U, class R =
        std::is_constructible<U, std::uint64_t>>
    static R check1(int);
    template<class>
    static std::false_type check1(...);
    using type1 = decltype(check1<T>(0));

    template<class U, class R =
        std::is_convertible<decltype(
            std::declval<U const>().operator()(
                std::declval<void const*>(),
                std::declval<std::size_t>())),
            std::uint64_t>>
    static R check2(int);
    template<class>
    static std::false_type check2(...);
    using type2 = decltype(check2<T>(0));
public:
    using type = std::integral_constant<bool,
        type1::value && type2::value>;
};

template<class T>
class is_Progress
{
    template<class U, class R = decltype(
        std::declval<U>().operator()(
            std::declval<std::uint64_t>(),
            std::declval<std::uint64_t>()),
                std::true_type{})>
    static R check1(int);
    template<class>
    static std::false_type check1(...);
public:
    using type = decltype(check1<T>(0));
};

} // detail

/// Determine if `T` meets the requirements of `Hasher`
template<class T>
#if GENERATING_DOCS
struct is_Hasher : std::integral_constant<bool, ...>{};
#else
using is_Hasher = typename detail::is_Hasher<T>::type;
#endif

/// Determine if `T` meets the requirements of `Progress`
template<class T>
#if GENERATING_DOCS
struct is_Progress : std::integral_constant<bool, ...>{};
#else
using is_Progress = typename detail::is_Progress<T>::type;
#endif

} // nudb

#endif
