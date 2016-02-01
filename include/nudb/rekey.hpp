//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_REKEY_HPP
#define NUDB_REKEY_HPP

#include <nudb/error.hpp>
#include <nudb/file.hpp>
#include <cstddef>
#include <cstdint>

namespace nudb {

/** Create a new key file from a data file.

    This algorithm rebuilds a key file for the given data file.
    It works efficiently by iterating the data file multiple times.
    During the iteration, a contiguous block of the key file is
    rendered in memory, then flushed to disk when the iteration is
    complete. The size of this memory buffer is controlled by the
    bufferSize parameter, larger is better. The algorithm works
    the fastest when bufferSize is large enough to hold the entire
    key file in memory; only a single iteration of the data file
    is needed in this case.

    @tparam Hasher The hash function to use. This type must
    meet the requirements of @b HashFunction. The hash function
    must be the same as that used to create the database, or
    else an error is returned.

    @param dat_path The path to the data file.

    @param key_path The path to the key file.

    @param itemCount The number of items in the data file.

    @param bufferSize The number of bytes to allocate for the buffer.

    @param ec Set to the error if any occurred.

    @param progress A function which will be called periodically
    as the algorithm proceeds. The equivalent signature of the
    progress function must be:
    @code
    void progress(
        std::uint64_t amount,   // Amount of work done so far
        std::uint64_t total     // Total amount of work to do
    );
    @endcode
*/
// VFALCO Should this delete the key file on an error?
template<class Hasher, class Progress>
void
rekey(
    path_type const& dat_path,
    path_type const& key_path,
    std::uint64_t itemCount,
    std::size_t bufferSize,
    Progress& progress,
    error_code& ec);

} // nudb

#include <nudb/impl/rekey.ipp>

#endif
