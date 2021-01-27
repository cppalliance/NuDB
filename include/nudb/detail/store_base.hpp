//
// Copyright (c) 2019 Miguel Portilla (miguelportilla64 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_DETAIL_STORE_BASE_HPP
#define NUDB_DETAIL_STORE_BASE_HPP

namespace nudb {

class context;

namespace detail {

class store_base
{
protected:
    friend class nudb::context;

    virtual void flush() = 0;

private:
#if ! NUDB_DOXYGEN
    friend class test::context_test;
#endif

    enum class state
    {
        waiting,
        flushing,
        intermediate,
        none
    };

    store_base* next_ = nullptr;
    store_base* prev_ = nullptr;
    state state_ = state::none;
};

} // detail
} // nudb

#endif
