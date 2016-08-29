//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Test that header file is self-contained
#include <nudb/store.hpp>

#include "test_util.hpp"
#include <beast/unit_test/suite.hpp>
#include <cmath>
#include <functional>
#include <iomanip>
#include <memory>
#include <random>
#include <utility>

namespace nudb {
namespace test {

// Basic, single threaded test that verifies the
// correct operation of the store. Load factor is
// set high to ensure that spill records are created,
// exercised, and split.
//
class store_test : public beast::unit_test::suite
{
public:
    void
    do_errors()
    {
        temp_dir td;
        error_code ec;
        auto const dp = td.file ("nudb.dat");
        auto const kp = td.file ("nudb.key");
        auto const lp = td.file ("nudb.log");
        store db;
        // Fetch without open database
        try
        {
            key_type key = 0;
            db.fetch(&key,
                [](void const*, std::size_t)
                {
                }, ec);
            return fail();
        }
        catch(std::exception const&)
        {
            pass();
        }
        // Insert without open database
        try
        {
            key_type key = 0;
            std::uint64_t data = 0;
            db.insert(&key, &data, sizeof(data), ec);
            return fail();
        }
        catch(std::exception const&)
        {
            pass();
        }
        create<xxhasher>(dp, kp, lp, appnumValue,
            saltValue, sizeof(key_type), block_size(dp),
                0.5, ec);
        if(! expect(! ec, ec.message()))
            return;
        finisher f(
            [&]
            {
                {
                    error_code ev;
                    native_file::erase(dp, ev);
                    expect(! ev, ev.message());
                }
                {
                    error_code ev;
                    native_file::erase(kp, ev);
                    expect(! ev, ev.message());
                }
                {
                    error_code ev;
                    erase_file(lp, ev);
                    expect(! ev, ev.message());
                }
            });
        db.open(dp, kp, lp, arenaAllocSize, ec);
        if(! expect(! ec, ec.message()))
            return;
        // Insert 0-size value
        try
        {
            key_type key = 0;
            std::uint64_t data = 0;
            db.insert(&key, &data, 0, ec);
            return fail();
        }
        catch(std::exception const&)
        {
            pass();
        }
        try
        {
            db.open(dp, kp, lp, arenaAllocSize, ec);
            return fail();
        }
        catch(std::exception const&)
        {
            pass();
        }
        db.close(ec);
        if(! expect(! ec, ec.message()))
            return;
        db.open(td.file("dat.dat"), kp, lp, arenaAllocSize, ec);
        if(! expect(ec == errc::no_such_file_or_directory))
            return;
    }

    void
    do_inserts(std::size_t N,
        nsize_t block_size, float load_factor)
    {
        temp_dir td;
        error_code ec;
        auto const dp = td.file ("nudb.dat");
        auto const kp = td.file ("nudb.key");
        auto const lp = td.file ("nudb.log");
        Sequence seq;
        store db;
        create<xxhasher>(dp, kp, lp, appnumValue,
            saltValue, sizeof(key_type), block_size,
                load_factor, ec);
        if(! expect(! ec, ec.message()))
            return;
        finisher f(
            [&]
            {
                {
                    error_code ev;
                    native_file::erase(dp, ev);
                    expect(! ev, ev.message());
                }
                {
                    error_code ev;
                    native_file::erase(kp, ev);
                    expect(! ev, ev.message());
                }
                {
                    error_code ev;
                    erase_file(lp, ev);
                    expect(! ev, ev.message());
                }
            });
        db.open(dp, kp, lp, arenaAllocSize, ec);
        if(! expect(! ec, ec.message()))
            return;
        Storage s;
        // insert
        for(std::size_t i = 0; i < N; ++i)
        {
            auto const v = seq[i];
            db.insert(&v.key, v.data, v.size, ec);
            if(! expect(! ec, ec.message()))
                return;
        }
        // fetch
        for(std::size_t i = 0; i < N; ++i)
        {
            auto const v = seq[i];
            db.fetch (&v.key, s, ec);
            if(! expect(! ec, ec.message()))
                return;
            expect(s.size() == v.size);
            expect(std::memcmp(s.get(),
                v.data, v.size) == 0);
        }
        // insert duplicates
        for(std::size_t i = 0; i < N; ++i)
        {
            auto const v = seq[i];
            db.insert(&v.key, v.data, v.size, ec);
            if(! expect(ec == error::key_exists, ec.message()))
                return;
            ec = {};
        }
        // insert/fetch
        for(std::size_t i = 0; i < N; ++i)
        {
            auto v = seq[i];
            db.fetch (&v.key, s, ec);
            if(! expect(! ec, ec.message()))
                return;
            expect(s.size() == v.size);
            expect(memcmp(s.get(), v.data, v.size) == 0);
            v = seq[i + N];
            db.insert(&v.key, v.data, v.size, ec);
            if(! expect(! ec, ec.message()))
                return;
        }
        db.close(ec);
        if(! expect(! ec, ec.message()))
            return;
        verify_info info;
        verify<xxhasher>(
            info, dp, kp, 0, no_progress{}, ec);
        if(! expect(! ec, ec.message()))
            return;
        expect(info.hist[1] > 0);
        log << info;
    }

    void
    run() override
    {
        enum
        {
            N =             50000,
            block_size =    256
        };

        float const load_factor = 0.95f;

        do_errors();
        do_inserts(N, block_size, load_factor);
    }
};

BEAST_DEFINE_TESTSUITE(store, test, nudb);

} // test
} // nudb
