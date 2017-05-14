//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//[ simple_example
#include <boost/system/error_code.hpp>

#include <nudb/xxhasher.hpp>    // xxhasher
#include <nudb/create.hpp>      // create
#include <nudb/store.hpp>       // store
#include <nudb/file.hpp>        // path_type

#include <iostream>
#include <cstdint>      // uint32_t, uint64_t
#include <utility>      // pair
#include <string>       // string
#include <algorithm>    // fill, copy_n, min
#include <iostream>

int main(){
    // error code returned by NuDb operations
    boost::system::error_code ec;

    // key type for this example - a social security number
    using ssn = std::uint64_t;

    // (1) File Names
    const nudb::path_type dat_path = "db.dat";
    const nudb::path_type key_path = "db.key";
    const nudb::path_type log_path = "db.log";

    // (2) Create a new database
    // given names of data, key and log files
    nudb::create<nudb::xxhasher>(
        dat_path,           // path name of data file
        key_path,           // path name of key file
        log_path,           // path name of log file
        1,                  // application number
        nudb::make_salt(),  // random seed
        sizeof(ssn),
        nudb::block_size("."), // block size of current directory
        0.5f,               // load factor
        ec                  // reference to return code
    );
    if(ec){
        std::cerr << "creation failed:" << ec.message() << std::endl;
        return 1;
    }
    std::cerr << "creation successful" << '\n';

    // (3) Open an existing database
    nudb::store db;
    db.open(dat_path, key_path, log_path, ec);
    if(ec){
        std::cerr << "open failed:" << ec.message() << std::endl;
        return 1;
    }
    std::cerr << "open successful" << '\n';

    const std::pair<ssn, const char *> input_data[] = {
        {123456789L, "bob"},
        {999999999L, "carol"},
        {987654321L, "ted"},
        {666666666L, "alice"}
    };

    // (4) Insert key/value pairs
    // insert ssn/name pairs
    for(const auto & p : input_data){
        db.insert(& p.first, p.second, std::strlen(p.second), ec);
        if(ec){
            std::cerr << "insertion failed:" << ec.message() << std::endl;
            return 1;
        }
    }
    std::cerr << "inserted 4 records" << '\n';

    // (5) Fetch a value given it's key
    // get carol's address
    ssn key = 999999999L;
    std::string address;
    db.fetch(
        & key,
        [&](void const * buffer, std::size_t size){
            address = std::string(static_cast<const char *>(buffer), size);
        },
        ec
    );
    if(ec){
        std::cerr << "fetch failed:" << ec.message() << std::endl;
        return 1;
    }
    std::cerr
        << "given ssn=" << key << ", "
        << "retrieved " << address << '\n';

    // (6) Terminate access to the database
    db.close(ec);
    if(ec){
        std::cerr << "close failed:" << ec.message() << std::endl;
        return 1;
    }
    std::cerr << "close successful" << '\n';

    return 0;
}
//]
