//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_BASIC_STORE_HPP
#define NUDB_BASIC_STORE_HPP

#include <nudb/file.hpp>
#include <nudb/type_traits.hpp>
#include <nudb/detail/cache.hpp>
#include <nudb/detail/gentex.hpp>
#include <nudb/detail/pool.hpp>
#include <boost/optional.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <chrono>
#include <mutex>
#include <thread>

namespace nudb {

/** A simple key/value database

    @tparam Hasher The hash function to use on key

    @tparam File The type of File object to use.
*/
template<class Hasher, class File>
class basic_store
{
public:
    using hash_type = Hasher;
    using file_type = File;

private:
    using clock_type =
        std::chrono::steady_clock;

    using shared_lock_type =
        boost::shared_lock<boost::shared_mutex>;

    using unique_lock_type =
        boost::unique_lock<boost::shared_mutex>;

    struct state
    {
        File df;
        File kf;
        File lf;
        path_type dp;
        path_type kp;
        path_type lp;
        Hasher hasher;
        detail::pool p0;
        detail::pool p1;
        detail::cache c0;
        detail::cache c1;
        detail::key_file_header kh;

        // pool commit high water mark
        std::size_t pool_thresh = 1;

        state(state const&) = delete;
        state& operator=(state const&) = delete;

        state(state&&) = default;
        state& operator=(state&&) = default;

        state(File&& df_, File&& kf_, File&& lf_,
            path_type const& dp_, path_type const& kp_,
                path_type const& lp_,
                    detail::key_file_header const& kh_,
                        std::size_t arenaBlockSize);
    };

    bool open_ = false;

    // Use optional because some
    // members cannot be default-constructed.
    //
    boost::optional<state> s_;      // State of an open database

    std::size_t frac_;              // accumulates load
    std::size_t thresh_;            // split threshold
    nbuck_t buckets_;               // number of buckets
    nbuck_t modulus_;               // hash modulus

    std::mutex u_;                  // serializes insert()
    detail::gentex g_;
    boost::shared_mutex m_;
    std::thread thread_;
    std::condition_variable_any cond_;

    // These allow insert to block, preventing the pool
    // from exceeding a limit. Currently the limit is
    // baked in, and can only be reached during sustained
    // insertions, such as while importing.
    std::size_t commit_limit_ = 1UL * 1024 * 1024 * 1024;
    std::condition_variable_any cond_limit_;

    error_code ec_;
    std::atomic<bool> ecb_;         // `true` when ec_ set

    std::size_t dataWriteSize_;
    std::size_t logWriteSize_;

public:
    basic_store() = default;
    basic_store(basic_store const&) = delete;
    basic_store& operator=(basic_store const&) = delete;

    /** Destroy the database.

        Files are closed, memory is freed, and data that has not been
        committed is discarded. To ensure that all inserted data is
        written, it is necessary to call close() before destroying the
        basic_store.

        This function catches all exceptions thrown by callees, so it
        will be necessary to call close() before destroying the basic_store
        if callers want to catch exceptions.

        Throws:
            None
    */
    ~basic_store();

    /** Returns `true` if the database is open.

        Thread safety:

    */
    bool
    is_open() const
    {
        return open_;
    }

    /** Return the path to the data file.

        Preconditions:
            The database must be open.

        @return The data file path.

        @throws std::logic_error if the database is not open.
    */
    path_type const&
    dat_path() const;

    /** Return the path to the key file.

        Preconditions:
            The database must be open.

        @return The key file path.

        @throws std::logic_error if the database is not open.
    */
    path_type const&
    key_path() const;

    /** Return the path to the log file.

        Preconditions:
            The database must be open.

        @return The log file path.

        @throws std::logic_error if the database is not open.
    */
    path_type const&
    log_path() const;

    /** Return the appnum associated with the database.

        Preconditions:
            The database must be open.

        @return The appnum.

        @throws std::logic_error if the database is not open.
    */
    std::uint64_t
    appnum() const;

    /** Return the key size associated with the database.

        Preconditions:
            The database must be open.

        @return The size of keys in the database.

        @throws std::logic_error if the database is not open.
    */
    std::size_t
    key_size() const;

    /** Return the block size associated with the database.

        Preconditions:
            The database must be open.

        @return The size of blocks in the key file.

        @throws std::logic_error if the database is not open.
    */
    std::size_t
    block_size() const;

    /** Close the database.

        All data is committed before closing.

        If an error occurs, the database is still closed.

        @param ec Set to the error, if any occurred.
    */
    void
    close(error_code& ec);

    /** Open a database.

        The database identified by the specified data, key, and
        log file paths is opened. If a log file is present, the
        recovery mechanism is invoked to restore database integrity
        before the function returns.

        @param dat_path The path to the data file.

        @param key_path The path to the key file.

        @param log_path The path to the log file.

        @param arenaBlockSize A hint to the size of the blocks
        used to allocate memory for buffering insertions. A reasonable
        value is one thousand times the size of the average value.

        @param ec Set to the error, if any occurred.

        @param args Optional arguments passed to File constructors.
        
    */
    template<class... Args>
    void
    open(
        path_type const& dat_path,
        path_type const& key_path,
        path_type const& log_path,
        std::size_t arenaBlockSize,
        error_code& ec,
        Args&&... args);

    /** Fetch a value.

        The function checks the database for the specified
        key, and invokes the callback if it is found. If
        the key is not found, `ec` is set to @ref error::key_not_found.
        If any other errors occur, `ec` is set to the
        corresponding error.

        @note If the implementation encounters an error while
        committing data to the database, this function will
        immediately return with `ec` set to the error which
        occurred. All subsequent calls to @ref fetch will
        return the same error until the database is closed.

        @param callback A function which will be called with the
        value data if the fetch is successful. The equivalent
        signature must be:
        @code
        void callback(
            void const* buffer, // A buffer holding the value
            std::size_t size    // The size of the value in bytes
        );
        @endcode
        The buffer provided to the callback remains valid
        until the callback returns, ownership is not transferred.

        @param ec Set to the error, if any occurred.

        @throws `std::logic_error` if the database is not open.
    */
    template<class Callback>
    void
    fetch(void const* key, Callback && callback, error_code& ec);

    /** Insert a value.

        This function attempts to insert the specified key/value
        pair into the database. If the key already exists,
        `ec` is set to @ref error::key_exists. If an error
        occurs, `ec` is set to the corresponding error.

        @note If the implementation encounters an error while
        committing data to the database, this function will
        immediately return with `ec` set to the error which
        occurred. All subsequent calls to @ref insert will
        return the same error until the database is closed.

        @param key A buffer holding the key to be inserted. The
        size of the buffer should be at least the `key_size`
        associated with the open database.

        @param data A buffer holding the value to be inserted.

        @param bytes The size of the buffer holding the value data.

        @param ec Set to the error, if any occurred.

        @throws `std::logic_error` if the database is not open,
        `std::domain_error` if the size is out of the allowable range.
    */
    void
    insert(void const* key, void const* data,
        nsize_t bytes, error_code& ec);

private:
    template<class Callback>
    void
    fetch(detail::nhash_t h, void const* key,
        detail::bucket b, Callback && callback, error_code& ec);

    bool
    exists(detail::nhash_t h, void const* key,
        shared_lock_type* lock, detail::bucket b, error_code& ec);

    void
    split(detail::bucket& b1, detail::bucket& b2,
        detail::bucket& tmp, nbuck_t n1, nbuck_t n2,
            nbuck_t buckets, nbuck_t modulus,
                detail::bulk_writer<File>& w, error_code& ec);

    detail::bucket
    load(nbuck_t n, detail::cache& c1,
        detail::cache& c0, void* buf, error_code& ec);

    void
    commit(error_code& ec);

    void
    run();
};

} // nudb

#include <nudb/impl/basic_store.ipp>

#endif
