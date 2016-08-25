//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

// Test that header file is self-contained
#include <nudb/rekey.hpp>

#include "test_util.hpp"
#include <nudb/progress.hpp>
#include <nudb/verify.hpp>
#include <beast/unit_test/suite.hpp>

namespace nudb {
namespace test {

// Simple test to check that rekey works, and
// also to exercise all its failure paths.
//
class rekey_test : public beast::unit_test::suite
{
public:
    void
    do_rekey(
        std::size_t N, nsize_t block_size, float load_factor)
    {
        auto const keys = static_cast<std::size_t>(
            load_factor * detail::bucket_capacity(block_size));
        std::size_t const bufferSize =
            block_size * (1 + ((N + keys - 1) / keys));

        temp_dir td;
        error_code ec;
        auto const dp  = td.file ("nudb.dat");
        auto const kp  = td.file ("nudb.key");
        auto const kp2 = td.file ("nudb.key2");
        auto const lp  = td.file ("nudb.log");
        finisher f(
            [&]
            {
                {
                    error_code ev;
                    native_file::erase(dp, ev);
                }
                {
                    error_code ev;
                    native_file::erase(kp, ev);
                }
                {
                    error_code ev;
                    native_file::erase(lp, ev);
                }
            });
        {
            Sequence seq;
            store db;
            create<xxhasher>(dp, kp, lp, appnumValue,
                saltValue, sizeof(key_type), block_size,
                    load_factor, ec);
            if(! expect(! ec, ec.message()))
                return;
            db.open(dp, kp, lp, arenaAllocSize, ec);
            if(! expect(! ec, ec.message()))
                return;
            Storage s;
            // Insert
            for(std::size_t i = 0; i < N; ++i)
            {
                auto const v = seq[i];
                auto const success =
                    db.insert(&v.key, v.data, v.size, ec);
                if(! expect(! ec, ec.message()))
                    return;
                expect(success);
            }
        }
        // Verify
        {
            verify_info info;
            verify<xxhasher>(
                info, dp, kp, bufferSize, no_progress{}, ec);
            if(! expect(! ec, ec.message()))
                return;
            if(! expect(info.value_count == N))
                return;
            if(! expect(info.spill_count > 0))
                return;
        }
        // Rekey
        for(std::size_t n = 1;; ++n)
        {
            fail_counter fc{n};
            rekey<xxhasher, fail_file<native_file>>(
                dp, kp2, lp, N, bufferSize, ec, no_progress{}, fc);
            if(! ec)
                break;
            if(! expect(ec == test::test_error::failure, ec.message()))
                return;
            ec = {};
            recover<xxhasher, native_file>(dp, kp2, lp, ec);
            if(! expect(! ec ||
                ec == errc::no_such_file_or_directory,
                    ec.message()))
                return;
            native_file::erase(kp2, ec);
            ec = {};
            verify_info info;
            verify<xxhasher>(
                info, dp, kp, bufferSize, no_progress{}, ec);
            if(! expect(! ec, ec.message()))
                return;
            if(! expect(info.value_count == N))
                return;
        }
        // Verify
        {
            verify_info info;
            verify<xxhasher>(
                info, dp, kp2, bufferSize, no_progress{}, ec);
            if(! expect(! ec, ec.message()))
                return;
            if(! expect(info.value_count == N))
                return;
        }
    }

    void
    do_recover(
        std::size_t N, nsize_t block_size, float load_factor)
    {
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

        do_rekey(N, block_size, load_factor);
        do_recover(N, block_size, load_factor);
    }
};

BEAST_DEFINE_TESTSUITE(rekey, test, nudb);

} // test
} // nudb
