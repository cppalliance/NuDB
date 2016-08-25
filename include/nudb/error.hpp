//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_ERROR_HPP
#define NUDB_ERROR_HPP

#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>

namespace nudb {

using boost::system::error_code;
using boost::system::error_condition;
using boost::system::system_error;
using boost::system::system_category;
using boost::system::generic_category;
using boost::system::error_category;
namespace errc = boost::system::errc;

/// Database error codes.
enum class error
{
    /** No error.

        The operation completed successfully.
    */
    success = 0,

    /** A file read returned less data than expected.

        This can be caused by premature application
        termination during a commit cycle.
    */
    short_read,

    /** A file write stored less data than expected.

        This is typically due to running out of space
        on a device holding a database file.
    */
    short_write,

    /** A log file is present.

        Indicates that the database needs to have the
        associated log file applied to perform a recovery.
        This error is returned by functions such as @ref rekey.
    */
    log_file_exists,

    /** No key file exists.

        This error is returned by the recover process when
        there is no valid key file. It happens when a
        @ref rekey operation prematurely terminates. A
        database without a key file cannot be opened. To
        fix this error, it is necessary for an invocation of
        @ref rekey to complete successfully.
    */
    no_key_file,
        


    /// Not a data file
    not_data_file,

    /// Not a key file
    not_key_file,

    /// Not a log file
    not_log_file,

    /// Different version
    different_version,

    /// Invalid key size
    invalid_key_size,

    /// Invalid block size
    invalid_block_size,

    /// Short key file
    short_key_file,

    /// Short bucket
    short_bucket,

    /// Short spill
    short_spill,

    /// Short record
    short_data_record,

    /// Short value
    short_value,

    /// Hash mismatch
    hash_mismatch,

    /// Invalid load factor
    invalid_load_factor,

    /// Invalid capacity
    invalid_capacity,

    /// Invalid bucket count
    invalid_bucket_count,

    /// Invalid bucket size
    invalid_bucket_size,

    /// The data file header was incomplete
    incomplete_data_file_header,

    /// The key file header was incomplete
    incomplete_key_file_header,

    /// Invalid log record
    invalid_log_record,

    /// Invalid spill in log record
    invalid_log_spill,

    /// Invalid offset in log record
    invalid_log_offset,

    /// Invalid index in log record
    invalid_log_index,

    /// Invalid size in spill
    invalid_spill_size,

    /// UID mismatch
    uid_mismatch,

    /// appnum mismatch
    appnum_mismatch,

    /// key size mismatch
    key_size_mismatch,

    /// salt mismatch
    salt_mismatch,

    /// pepper mismatch
    pepper_mismatch,

    /// block size mismatch
    block_size_mismatch,

    /// orphaned value
    orphaned_value,

    /// missing value
    missing_value,

    /// size mismatch
    size_mismatch,

    /// duplicate value
    duplicate_value
};

/// Returns the error category used for database error codes.
error_category const&
nudb_category();

/// Returns a database error code.
inline
error_code
make_error_code(error ev)
{
    return error_code{static_cast<int>(ev), nudb_category()};
}

} // nudb

namespace boost {
namespace system {
template<>
struct is_error_code_enum<nudb::error>
{
    static bool const value = true;
};
} // system
} // boost

#include <nudb/impl/error.ipp>

#endif
