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
    do_test (std::size_t N,
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
            auto const success =
                db.insert(&v.key, v.data, v.size, ec);
            if(! expect(! ec, ec.message()))
                return;
            expect(success);
        }
        // fetch
        for(std::size_t i = 0; i < N; ++i)
        {
            auto const v = seq[i];
            bool const found = db.fetch (&v.key, s, ec);
            if(! expect(! ec, ec.message()))
                return;
            expect(found);
            expect(s.size() == v.size);
            expect(std::memcmp(s.get(),
                v.data, v.size) == 0);
        }
        // insert duplicates
        for(std::size_t i = 0; i < N; ++i)
        {
            auto const v = seq[i];
            auto const success = db.insert(
                &v.key, v.data, v.size, ec);
            if(! expect(! ec, ec.message()))
                return;
            expect(! success);
        }
        // insert/fetch
        for(std::size_t i = 0; i < N; ++i)
        {
            auto v = seq[i];
            bool const found = db.fetch (&v.key, s, ec);
            if(! expect(! ec, ec.message()))
                return;
            expect(found);
            expect(s.size() == v.size);
            expect(memcmp(s.get(), v.data, v.size) == 0);
            v = seq[i + N];
            auto const success = db.insert(
                &v.key, v.data, v.size, ec);
            if(! expect(! ec, ec.message()))
                return;
            expect(success);
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
        print(log, info);
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

        do_test (N, block_size, load_factor);
    }
};

BEAST_DEFINE_TESTSUITE(store, test, nudb);

} // test
} // nudb
