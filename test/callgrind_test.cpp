//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "test_util.hpp"

#include <nudb/create.hpp>
#include <nudb/native_file.hpp>
#include <beast/unit_test/suite.hpp>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

namespace nudb {
namespace test {

// This test is designed for callgrind runs to find hotspots

class callgrind_test : public beast::unit_test::suite
{
public:
    // Creates and opens a database, performs a bunch
    // of inserts, then alternates fetching all the keys
    // with keys not present.
    //
    void
    testCallgrind(std::size_t count, path_type const& path)
    {
        error_code ec;
        auto const dp = path + ".dat";
        auto const kp = path + ".key";
        auto const lp = path + ".log";
        create<xxhasher>(dp, kp, lp,
            appnumValue,
            saltValue,
            sizeof(key_type),
            nudb::block_size(path),
            0.50,
            ec);
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
                    native_file::erase(lp, ev);
                    expect(ev == errc::no_such_file_or_directory, ev.message());
                }
            });
        store db;
        db.open(dp, kp, lp, arenaAllocSize, ec);
        if(! expect(! ec, ec.message()))
            return;
        expect(db.appnum() == appnumValue, "appnum");
        Sequence seq;
        for(std::size_t i = 0; i < count; ++i)
        {
            auto const v = seq[i];
            auto const success = db.insert(
                &v.key, v.data, v.size, ec);
            if(! expect(! ec, ec.message()))
                return;
            expect(success);
        }
        Storage s;
        for(std::size_t i = 0; i < count * 2; ++i)
        {
            if(! (i%2))
            {
                auto const v = seq[i/2];
                expect (db.fetch(&v.key, s, ec), "fetch");
                if(! expect(! ec, ec.message()))
                    return;
                expect (s.size() == v.size, "size");
                expect (std::memcmp(s.get(),
                    v.data, v.size) == 0, "data");
            }
            else
            {
                auto const v = seq[count + i/2];
                auto const found = db.fetch (&v.key, s, ec);
                if(! expect(! ec, ec.message()))
                    return;
                expect(! found);
            }
        }
        db.close(ec);
        if(! expect(! ec, ec.message()))
            return;
    }

    void run()
    {
        // higher numbers, more pain
        static std::size_t constexpr N = 100000;

        temp_dir td;
        testCallgrind(N, td.path());
    }
};

BEAST_DEFINE_TESTSUITE(callgrind, test, nudb);

} // test
} // nudb
