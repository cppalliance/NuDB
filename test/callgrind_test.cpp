//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "suite.hpp"
#include "test_util.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <utility>

namespace nudb {
namespace test {

// This test is designed for callgrind runs to find hotspots

class callgrind_test : public suite
{
public:
    // Creates and opens a database, performs a bunch
    // of inserts, then alternates fetching all the keys
    // with keys not present.
    //
    void
    testCallgrind(std::size_t count, path_type const& path)
    {
        auto const dp = path + ".dat";
        auto const kp = path + ".key";
        auto const lp = path + ".log";
        test_api::create(dp, kp, lp,
            appnum,
            salt,
            sizeof(nudb::test::key_type),
            nudb::block_size(path),
            0.50);
        test_api::store db;
        if(! expect (db.open(dp, kp, lp,
                arena_alloc_size), "open"))
            return;
        expect (db.appnum() == appnum, "appnum");
        Sequence seq;
        for(std::size_t i = 0; i < count; ++i)
        {
            auto const v = seq[i];
            expect (db.insert(&v.key, v.data, v.size),
                "insert");
        }
        Storage s;
        for(std::size_t i = 0; i < count * 2; ++i)
        {
            if(! (i%2))
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
        nudb::native_file::erase(dp);
        nudb::native_file::erase(kp);
        nudb::native_file::erase(lp);
    }

    void run()
    {
        // higher numbers, more pain
        static std::size_t constexpr N = 100000;

        temp_dir td;
        testCallgrind(N, td.path());
    }
};

} // test
} // nudb

int main()
{
    std::cout << "callgrind_test:" << std::endl;
    nudb::test::callgrind_test t;
    return t(std::cerr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
