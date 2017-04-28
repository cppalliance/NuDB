//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//[ main
#include <boost/system/error_code.hpp>

#include <nudb/xxhasher.hpp>    // xxhasher
#include <nudb/create.hpp>      // create
#include <nudb/store.hpp>       // store
#include <nudb/file.hpp>        // path_type

#include <iostream>
#include <cstdint>              // std::uint32_t, std::uint64_t

// NuDB functions may return error_code defined by
bool is_success(const boost::system::error_code & ec){
    return
        // boost system library
        ec == boost::system::errc::success
        // or nudb error header
        || ec == nudb::error::success;
}

int main()
{
    boost::system::error_code ec;

    // (1) Files
    const nudb::path_type dat_path = "db.dat";
    const nudb::path_type key_path = "db.key";
    const nudb::path_type log_path = "db.log";

    // key type for this exampe
    using key_type = std::uint32_t;

    // given names of data, key and log files
    // (2) Create a new database
    nudb::create<nudb::xxhasher>(
        dat_path,           // path name of data file
        key_path,           // path name of key file
        log_path,           // path name of log file
        1,                  // application number
        nudb::make_salt(),        // randome seed
        sizeof(key_type),
        nudb::block_size("."),    // block size of current directory
        0.5f,               // load factor
        ec                  // reference to return code
    );
    if(! is_success(ec)){
        std::cerr << "creation failed:" << ec.message() << std::endl;
        return 1;
    }

    // (3) Open an existing database
    nudb::store db;
    db.open(dat_path, key_path, log_path, ec);
    if(! is_success(ec)){
        std::cerr << "open failed:" << ec.message() << std::endl;
        return 1;
    }

    // data is one byte long containing a zero value
    const char data = 0;
    // for each key value from 0 to 999
    for(key_type k = 0; k < 1000; ++k){
        // each with a key valued 0 to 1000
        // (4) Insert a key/value pair
        db.insert(&k, &data, sizeof(data), ec);
        if(! is_success(ec)){
            std::cerr << "insertion failed:" << ec.message() << std::endl;
            return 1;
        }
    }

    // Fetch each block back in order
    for(key_type k = 0; k < 1000; ++k){
        // (5) Fetch a value given it's key
        db.fetch(
            &k,
            [&](void const* buffer, std::size_t size){
                // verify that the size of the data is 1
                assert(sizeof(data) == size);
                // verify that the block contains a zero
                assert(0 == * static_cast<const char *>(buffer));
            },
            ec
        );
        if(! is_success(ec)){
            std::cerr << "fetch failed:" << ec.message() << std::endl;
            return 1;
        }
    }

    // (6) Terminate access to the database
    db.close(ec);
    if(! is_success(ec)){
        std::cerr << "close failed:" << ec.message() << std::endl;
        return 1;
    }

    // (7) Delete the databasae
    nudb::erase_file(dat_path);
    nudb::erase_file(key_path);
    nudb::erase_file(log_path);

    return 0;
}
//]
