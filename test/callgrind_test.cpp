//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "test_util.hpp"

#include <cmath>
#include <cstring>
#include <memory>
#include <random>
#include <utility>

namespace nudb {
namespace test {

// This test is designed for callgrind runs to find hotspots

// Creates and opens a database, performs a bunch
// of inserts, then alternates fetching all the keys
// with keys not present.
//
void
valgrind_test(std::size_t count, path_type const& path)
{
    auto const dp = path + ".dat";
    auto const kp = path + ".key";
    auto const lp = path + ".log";
    test_api::create (dp, kp, lp,
        appnum,
        salt,
        sizeof(nudb::test::key_type),
        nudb::block_size(path),
        0.50);
    test_api::store db;
    if (! expect (db.open(dp, kp, lp,
            arena_alloc_size), "open"))
        return;
    expect (db.appnum() == appnum, "appnum");
    Sequence seq;
    for (std::size_t i = 0; i < count; ++i)
    {
        auto const v = seq[i];
        expect (db.insert(&v.key, v.data, v.size),
            "insert");
    }
    Storage s;
    for (std::size_t i = 0; i < count * 2; ++i)
    {
        if (! (i%2))
        {
            auto const v = seq[i/2];
            expect (db.fetch (&v.key, s), "fetch");
            expect (s.size() == v.size, "size");
            expect (std::memcmp(s.get(),
                v.data, v.size) == 0, "data");
        }
        else
        {
            auto const v = seq[count + i/2];
            expect (! db.fetch (&v.key, s),
                "fetch missing");
        }
    }
    db.close();
    nudb::native_file::erase (dp);
    nudb::native_file::erase (kp);
    nudb::native_file::erase (lp);
}

int main()
{
    // higher numbers, more pain
    static std::size_t constexpr N = 100000;

    temp_dir td;
    valgrind_test(N, td.path());
}

} // test
} // nudb

