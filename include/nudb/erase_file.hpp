//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_ERASE_FILE_HPP
#define NUDB_ERASE_FILE_HPP

#include <nudb/error.hpp>
#include <nudb/file.hpp>
#include <nudb/native_file.hpp>

namespace nudb {

/** Erase a file if it exists.

    This function attempts to erase the specified file.
    No error is generated if the file does not already
    exist.

    @param path The path to the file to erase.

    @param ec Set to the error, if any occurred.

    @tparam File A type meeting the requirements of @b File.
    If this type is unspecified, @ref native_file is used.
*/
template<class File = native_file>
inline
void
erase_file(path_type const& path, error_code& ec)
{
    native_file::erase(path, ec);
    if(ec == errc::no_such_file_or_directory)
        ec = {};
}

/** Erase a file without returnign an error.

    This function attempts to erase the specified file.
    Any errors are ignored, including if the file does
    not exist.

    @param path The path to the file to erase.

    @tparam File A type meeting the requirements of @b File.
    If this type is unspecified, @ref native_file is used.
*/
template<class File = native_file>
inline
void
erase_file(path_type const& path)
{
    error_code ec;
    File::erase(path, ec);
}

} // nudb

#endif
