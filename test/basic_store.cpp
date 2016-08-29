//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Test that header file is self-contained
#include <nudb/basic_store.hpp>

#include <nudb/test/test_store.hpp>
#include <nudb/detail/arena.hpp>
#include <nudb/detail/cache.hpp>
#include <nudb/detail/pool.hpp>
#include <nudb/progress.hpp>
#include <nudb/verify.hpp>
#include <beast/unit_test/suite.hpp>
#include <limits>
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

namespace test {

class basic_store_test : public beast::unit_test::suite
{
public:
    // Inserts a bunch of values then fetches them
    void
    do_insert_fetch(
        std::size_t N,
        std::size_t keySize,
        std::size_t blockSize,
        float loadFactor,
        bool sleep)
    {
        testcase <<
            "N=" << N << ", "
            "keySize=" << keySize << ", "
            "blockSize=" << blockSize;
        error_code ec;
        test_store ts{keySize, blockSize, loadFactor};
        ts.create(ec);
        if(! BEAST_EXPECTS(! ec, ec.message()))
            return;
        ts.open(ec);
        if(! BEAST_EXPECTS(! ec, ec.message()))
            return;
        // Insert
        for(std::size_t n = 0; n < N; ++n)
        {
            auto const item = ts[n];
            ts.db.insert(item.key, item.data, item.size, ec);
            if(! BEAST_EXPECTS(! ec, ec.message()))
                return;
        }
        // Fetch
        for(std::size_t n = 0; n < N; ++n)
        {
            auto const item = ts[n];
            ts.db.fetch(item.key,
                [&](void const* data, std::size_t size)
                {
                    if(! BEAST_EXPECT(size == item.size))
                        return;
                    BEAST_EXPECT(
                        std::memcmp(data, item.data, size) == 0);
                }, ec);
        }
        // Insert Duplicate
        for(std::size_t n = 0; n < N; ++n)
        {
            auto const item = ts[n];
            ts.db.insert(item.key, item.data, item.size, ec);
            if(! BEAST_EXPECTS(
                    ec == error::key_exists, ec.message()))
                return;
            ec = {};
        }
        // Insert and Fetch
        if(keySize > 1)
        {
            for(std::size_t n = 0; n < N; ++n)
            {
                auto item = ts[n];
                ts.db.fetch(item.key,
                    [&](void const* data, std::size_t size)
                    {
                        if(! BEAST_EXPECT(size == item.size))
                            return;
                        BEAST_EXPECT(
                            std::memcmp(data, item.data, size) == 0);
                    }, ec);
                item = ts[N + n];
                ts.db.insert(item.key, item.data, item.size, ec);
                if(! BEAST_EXPECTS(! ec, ec.message()))
                    return;
                ts.db.fetch(item.key,
                    [&](void const* data, std::size_t size)
                    {
                        if(! BEAST_EXPECT(size == item.size))
                            return;
                        BEAST_EXPECT(
                            std::memcmp(data, item.data, size) == 0);
                    }, ec);
            }
        }
        if(sleep)
        {
            // Trigger a shrink_to_fit
            std::this_thread::sleep_for(
                std::chrono::milliseconds{2000});
        }
        ts.close(ec);
        if(! BEAST_EXPECTS(! ec, ec.message()))
            return;
    }

    // Perform insert/fetch test across a range of parameters
    void
    test_insert_fetch()
    {
        for(auto const keySize : {
            1, 2, 3, 31, 32, 33, 63, 64, 65, 95, 96, 97 })
        {
            std::size_t N;
            std::size_t constexpr blockSize = 4096;
            float loadFactor = 0.95f;
            switch(keySize)
            {
            case 1: N = 10; break;
            case 2: N = 100; break;
            case 3: N = 500; break;
            default:
                N = 5000;
                break;
            };
            do_insert_fetch(N, keySize, blockSize, loadFactor,
                keySize == 97);
        }
    }

    template<class F>
    void
    check_throw(F const& f)
    {
        try
        {
            f();
            BEAST_EXPECT(false);
        }
        catch(std::exception const&)
        {
            pass();
        }
        catch(...)
        {
            BEAST_EXPECT(false);
        }
    }

    // 32-bit std::size_t
    template<class = void>
    void
    test_throws(test_store&, std::false_type)
    {
    }

    // 64-bit std::size_t
    template<class = void>
    void
    test_throws(test_store& ts, std::true_type)
    {
        static_assert(sizeof(std::size_t) >= 8, "");
        std::size_t size =
            std::numeric_limits<std::size_t>::max() + 1;
        error_code ec;
        check_throw([&]{ ts.db.insert(nullptr, nullptr, size, ec); });
    }

    // Test that APIs throw when the db is not open
    void
    test_throws()
    {
        std::size_t const keySize = 4;
        std::size_t const blockSize = 4096;
        float loadFactor = 0.5f;

        error_code ec;
        test_store ts{keySize, blockSize, loadFactor};

        // Operations without open files
        check_throw([&]{ ts.db.dat_path(); });
        check_throw([&]{ ts.db.key_path(); });
        check_throw([&]{ ts.db.log_path(); });
        check_throw([&]{ ts.db.appnum(); });
        check_throw([&]{ ts.db.key_size(); });
        check_throw([&]{ ts.db.block_size(); });
        check_throw([&]{ ts.db.fetch(nullptr,
            [](void const*, std::size_t)
            {
            }, ec);});
        check_throw([&]{ ts.db.insert(
            nullptr, nullptr, 0, ec); });

        // Files not found
        ts.open(ec);
        if(! BEAST_EXPECTS(ec ==
                errc::no_such_file_or_directory, ec.message()))
            return;
        ec = {};

        ts.create(ec);
        if(! BEAST_EXPECTS(! ec, ec.message()))
            return;
        ts.open(ec);
        if(! BEAST_EXPECTS(! ec, ec.message()))
            return;
        BEAST_EXPECT(ts.dp == ts.db.dat_path());
        BEAST_EXPECT(ts.kp == ts.db.key_path());
        BEAST_EXPECT(ts.lp == ts.db.log_path());

        // Already open
        check_throw([&]{ ts.open(ec); });

        // Insert 0-size value
        check_throw([&]{ ts.db.insert(
            nullptr, nullptr, 0, ec); });

        // Insert value size too large
        test_throws(ts, std::integral_constant<bool,
            sizeof(std::size_t) >= 8>{});
    }

    void
    run() override
    {
        test_throws();
        test_insert_fetch();
    }
};

BEAST_DEFINE_TESTSUITE(basic_store, test, nudb);

} // test

} // nudb

