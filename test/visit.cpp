//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Test that header file is self-contained
#include <nudb/visit.hpp>

#include "test_util.hpp"
#include <beast/unit_test/suite.hpp>
#include <unordered_map>

namespace nudb {
namespace test {

class visit_test : public beast::unit_test::suite
{
public:
    void
    do_visit(std::size_t N, float loadFactor)
    {
        temp_dir td;
        error_code ec;
        auto const dp = td.file ("nudb.dat");
        auto const kp = td.file ("nudb.key");
        auto const lp = td.file ("nudb.log");
        create<xxhasher>(dp, kp, lp, appnumValue,
            saltValue, sizeof(key_type), block_size(dp),
                loadFactor, ec);
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
        Sequence seq;
        std::unordered_map<key_type, std::size_t> map;
        {
            store db;
            db.open(dp, kp, lp, arenaAllocSize, ec);
            if(! expect(! ec, ec.message()))
                return;
            Storage s;
            // Insert
            for(std::size_t i = 0; i < N; ++i)
            {
                auto const v = seq[i];
                map[v.key] = i;
                auto const success =
                    db.insert(&v.key, v.data, v.size, ec);
                if(! expect(! ec, ec.message()))
                    return;
                expect(success);
            }
        }
        // Visit
        visit(dp,
            [&](void const* key, std::size_t keySize,
                void const* data, std::size_t dataSize,
                error_code& ec)
            {
                auto const fail =
                    [&ec]
                    {
                        ec = error_code{
                            errc::invalid_argument, generic_category()};
                    };
                if(! expect(keySize == sizeof(key_type)))
                    return fail();
                // VFALCO This could fail on non intel since it
                //        could be doing an unaligned integer read.
                auto const& k =
                *reinterpret_cast<key_type const*>(key);
                auto const it = map.find(k);
                if(it == map.end())
                    return fail();
                auto const v = seq[it->second];
                if(! expect(dataSize == v.size))
                    return fail();
                auto const result =
                    std::memcmp(data, v.data, v.size);
                if(result != 0)
                    return fail();

            }, no_progress{}, ec);
        if(! expect(! ec, ec.message()))
            return;
    }

    void
    run() override
    {
        do_visit(5000, 0.95f);
    }
};

BEAST_DEFINE_TESTSUITE(visit, test, nudb);

} // test
} // nudb

