//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Test that header file is self-contained
#include <nudb/basic_store.hpp>

#include <nudb/detail/arena.hpp>
#include <nudb/detail/cache.hpp>
#include <nudb/detail/pool.hpp>
#include <type_traits>

namespace nudb {
namespace detail {

static_assert(!std::is_copy_constructible   <arena>{}, "");
static_assert(!std::is_copy_assignable      <arena>{}, "");
static_assert( std::is_move_constructible   <arena>{}, "");
static_assert(!std::is_move_assignable      <arena>{}, "");

static_assert(!std::is_copy_constructible   <cache>{}, "");
static_assert(!std::is_copy_assignable      <cache>{}, "");
static_assert( std::is_move_constructible   <cache>{}, "");
static_assert(!std::is_move_assignable      <cache>{}, "");

static_assert(!std::is_copy_constructible   <pool>{}, "");
static_assert(!std::is_copy_assignable      <pool>{}, "");
static_assert( std::is_move_constructible   <pool>{}, "");
static_assert(!std::is_move_assignable      <pool>{}, "");

} // detail
} // nudb

