//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_VERIFY_HPP
#define NUDB_VERIFY_HPP

#include <nudb/file.hpp>
#include <nudb/type_traits.hpp>
#include <nudb/detail/bucket.hpp>
#include <nudb/detail/bulkio.hpp>
#include <nudb/detail/format.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

namespace nudb {

/// Describes database statistics calculated by @ref verify.
struct verify_info
{
    int algorithm;                      // 0 = normal, 1 = fast
    path_type dat_path;                 // Path to data file
    path_type key_path;                 // Path to key file

    // Configured
    std::size_t version = 0;            // API version
    std::uint64_t uid = 0;              // UID
    std::uint64_t appnum = 0;           // Appnum
    nsize_t key_size = 0;               // Size of a key in bytes
    std::uint64_t salt = 0;             // Salt
    std::uint64_t pepper = 0;           // Pepper
    nsize_t block_size = 0;             // Block size in bytes
    float load_factor = 0;              // Target bucket fill fraction

    // Calculated
    nkey_t capacity = 0;                // Max keys per bucket
    nbuck_t buckets = 0;                // Number of buckets
    nsize_t bucket_size = 0;            // Size of bucket in bytes

    // Measured
    noff_t key_file_size = 0;           // Key file size in bytes
    noff_t dat_file_size = 0;           // Data file size in bytes
    std::uint64_t key_count = 0;        // Keys in buckets and active spills
    std::uint64_t value_count = 0;      // Count of values in the data file
    std::uint64_t value_bytes = 0;      // Sum of value bytes in the data file
    std::uint64_t spill_count = 0;      // used number of spill records
    std::uint64_t spill_count_tot = 0;  // Number of spill records in data file
    std::uint64_t spill_bytes = 0;      // used byte of spill records
    std::uint64_t spill_bytes_tot = 0;  // Sum of spill record bytes in data file

    // Performance
    float avg_fetch = 0;                // average reads per fetch (excluding value)
    float waste = 0;                    // fraction of data file bytes wasted (0..100)
    float overhead = 0;                 // percent of extra bytes per byte of value
    float actual_load = 0;              // actual bucket fill fraction

    // number of buckets having n spills
    std::array<nbuck_t, 10> hist;

    verify_info()
    {
        hist.fill(0);
    }
};

/** Verify consistency of the key and data files.

    This function opens the key and data files, and
    performs the following checks on the contents:

    @li Data file header validity

    @li Key file header validity

    @li Data and key file header agreements

    @li Check that each value is contained in a bucket

    @li Check that each bucket item reflects a value

    @li Ensure no values with duplicate keys

    Undefined behavior results when verifying a database
    that still has a log file. Use @ref recover on such
    databases first.

    This function selects one of two algorithms to use, the
    normal version, and a faster version that can take advantage
    of a buffer of sufficient size. Depending on the value of
    the bufferSize argument, the appropriate algorithm is chosen.

    A good value of bufferSize is one that is a large fraction
    of the key file size. For example, 20% of the size of the
    key file. Larger is better, with the highest usable value
    depending on the size of the key file. If presented with
    a buffer size that is too large to be of extra use, the
    fast algorithm will simply allocate what it needs.

    @tparam Hasher The hash function to use. This type must
    meet the requirements of @b HashFunction. The hash function
    must be the same as that used to create the database, or
    else an error is returned.

    @param dat_path The path to the data file.

    @param key_path The path to the key file.

    @param bufferSize The number of bytes to allocate for the buffer.
    If this number is too small, or zero, a slower algorithm will be
    used that does not require a buffer.

    @param progress A function which will be called periodically
    as the algorithm proceeds. The equivalent signature of the
    progress function must be:
    @code
    void progress(
        std::uint64_t amount,   // Amount of work done so far
        std::uint64_t total     // Total amount of work to do
    );
    @endcode

    @param ec Set to the error, if any occurred.
*/
template<class Hasher, class Progress>
void
verify(
    verify_info& info,
    path_type const& dat_path,
    path_type const& key_path,
    std::size_t bufferSize,
    Progress&& progress,
    error_code& ec);

} // nudb

#include <nudb/impl/verify.ipp>

#endif
