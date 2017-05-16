//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//[ crash

// simulates system crash while in use.

#include <boost/system/error_code.hpp>

#include <nudb/store.hpp>       // store

#include <iostream>
#include <cstdint>  // std::uint32_t, std::uint64_t
#include <cstdlib>  // std::abort

int main(){
    boost::system::error_code ec;

    // key type for this example
    using ssn = std::uint64_t;

    // Open an existing database
    nudb::store db;
    db.open("db.dat", "db.key", "db.log", ec);
    if(ec){
        std::cerr << "open failed: " << ec.message() << std::endl;
        return 1;
    }
    std::cerr << "open successful" << '\n';

    ssn key = 777777777L;
    const char * name = "george";

    // insert a ssn/name pair
    db.insert(& key, name, std::strlen(name), ec);

    // simulate a crash
    std::abort();

    return 1;
}
//]
