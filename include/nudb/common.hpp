//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_COMMON_HPP
#define NUDB_COMMON_HPP

#include <stdexcept>
#include <string>

namespace nudb {

// Commonly used types

enum class file_mode
{
    scan,         // read sequential
    read,         // read random
    append,       // read random, write append
    write         // read random, write random
};

using path_type = std::string;

// All exceptions thrown by nudb are derived
// from std::runtime_error except for fail_error

/** Thrown when a codec fails, e.g. corrupt data. */
struct codec_error : std::runtime_error
{
    template <class String>
    explicit
    codec_error (String const& s)
        : runtime_error(s)
    {
    }
};

/** Base class for all errors thrown by file classes. */
struct file_error : std::runtime_error
{
    template <class String>
    explicit
    file_error (String const& s)
        : runtime_error(s)
    {
    }
};

/** Thrown when file bytes read are less than requested. */
struct file_short_read_error : file_error
{
    file_short_read_error()
        : file_error (
            "nudb: short read")
    {
    }
};

/** Thrown when file bytes written are less than requested. */
struct file_short_write_error : file_error
{
    file_short_write_error()
        : file_error (
            "nudb: short write")
    {
    }
};

/** Thrown when end of istream reached while reading. */
struct short_read_error : std::runtime_error
{
    short_read_error()
        : std::runtime_error(
            "nudb: short read")
    {
    }
};

/** Base class for all exceptions thrown by store. */
class store_error : public std::runtime_error
{
public:
    template <class String>
    explicit
    store_error (String const& s)
        : runtime_error(s)
    {
    }
};

/** Thrown when corruption in a file is detected. */
class store_corrupt_error : public store_error
{
public:
    template <class String>
    explicit
    store_corrupt_error (String const& s)
        : store_error(s)
    {
    }
};

} // nudb

#endif
