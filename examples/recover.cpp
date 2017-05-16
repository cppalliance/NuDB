//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//[ recover
#include <boost/system/error_code.hpp>

#include <nudb/xxhasher.hpp>    // xxhasher
#include <nudb/create.hpp>      // create
#include <nudb/store.hpp>       // store
#include <nudb/visit.hpp>
#include <nudb/progress.hpp>

#include <iostream>
#include <cstdint>  // std::uint32_t, std::uint64_t

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

    nudb::visit(
        "db.dat",
        [&](// called with each item found in the data file
            void const* key,                // A pointer to the item key
            std::size_t key_size,           // The size of the key (always the same)
            void const* data,               // A pointer to the item data
            std::size_t data_size,          // The size of the item data
            boost::system::error_code& ec   // Indicates an error (out parameter)
        ){
            if(ec){
                std::cerr << "visit failed: " << ec.message() << std::endl;
                return std::terminate();
            }
            std::cerr
                << "key: " << * static_cast<const ssn *>(key) << '\n'
                << "name: " <<
                    std::string(static_cast<const char *>(data), data_size) << '\n'
            ;
        },
        [&](// called to indicate progress of visitation
            std::uint64_t amount,   // Amount of work done so far
            std::uint64_t total     // Total amount of work to do
        ){
            // we ignore this information in this example
        },
        ec  // result of visit operation
    );
    if(ec){
        std::cerr << "visit failed: " << ec.message() << std::endl;
        return 1;
    }
    return 0;
}

//]
