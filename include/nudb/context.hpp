//
// Copyright (c) 2019 Miguel Portilla (miguelportilla64 at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_CONTEXT_HPP
#define NUDB_CONTEXT_HPP

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

#define NUDB_DECL inline

namespace nudb {

namespace test {
    class context_test;
} // test

namespace detail {
    class store_base;
} // detail

class context
{
public:
    /// Constructor
    context() = default;

    /** Destructor.

        Halts execution of all threads. Blocks until all threads
        have completed any ongoing operation and have stopped.
    */
    NUDB_DECL
    ~context() noexcept (false);

    /// Copy constructor (disallowed)
    context(context const&) = delete;

    /// Move constructor (disallowed)
    context(context&&) = delete;

    // Copy assignment (disallowed)
    context& operator=(context const&) = delete;

    // Move assignment (disallowed)
    context& operator=(context&&) = delete;

    /** Start thread execution.

        Starts the context thread execution.
    */
    NUDB_DECL
    void
    start();

    /** Stop thread execution.

        Halts execution of all threads. Blocks until all threads
        have completed any ongoing operation and have stopped.
    */
    NUDB_DECL
    void
    stop_all();

    /** Thread function.

        Function to service the databases.
        Thread objects must call this function.
    */
    NUDB_DECL
    void
    run();

private:
    using clock_type = std::chrono::steady_clock;
    using store_base = detail::store_base;

    template<class, class> friend class basic_store;
#if ! NUDB_DOXYGEN
    friend class test::context_test;
#endif

    NUDB_DECL
    void
    insert(store_base& store);

    NUDB_DECL
    void
    erase(store_base& store);

    NUDB_DECL
    bool
    flush_one();

    class list
    {
    public:
        NUDB_DECL
        void
        push_back(store_base* node);

        NUDB_DECL
        void
        erase(store_base* node);

        NUDB_DECL
        void
        splice(list& l);

        bool
        empty() const noexcept
        {
            return head_ == nullptr;
        }

        store_base* head_ = nullptr;

    private:
        store_base* tail_ = nullptr;
    };

    std::mutex m_;
    list waiting_;
    std::condition_variable cv_w_;
    list flushing_;
    std::condition_variable cv_f_;

    std::uint32_t num_threads_ = 0;
    std::thread t_;

    bool stop_ = false;
};

} // nudb

#include <nudb/impl/context.ipp>

#endif
