//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//[ main
#include <nudb/nudb.hpp>
#include <cstddef>
#include <cstdint>

int main()
{
    //using namespace nudb;
    nudb::error_code ec;

    char const * const dat_path = "db.dat";
    char const * const key_path = "db.key";
    char const * const auto const log_path = "db.log";

    // given names of data, key and log files
    // create a new database
    nudb::create<xxhasher>(
        dat_path, key_path, log_path,   // path names
        1,                              // application number
        make_salt(),                    // randome seed
        sizeof(key_type),
        block_size("."),
        0.5f,                           // load factor
        ec                              // reference to return code
    );

    nudb::store db;
    db.open(dat_path, key_path, log_path, ec);

    char data = 0;
    // Insert 1000 blocks
    const std::size_t N = 1000;
    for(size_t i = 0; i < N; ++i)
        db.insert(&i, &data, sizeof(data), ec);

    // Fetch
    for(size_t i = 0; i < N; ++i)
        db.fetch(
            &i,
            [&](void const* buffer, std::size_t size){
                // do something with buffer, size
            },
            ec
        );

    db.close(ec);

    nudb::erase_file(dat_path);
    nudb::erase_file(key_path);
    nudb::erase_file(log_path);
}
//]
