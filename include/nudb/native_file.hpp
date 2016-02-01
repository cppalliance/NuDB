//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_NATIVE_FILE_HPP
#define NUDB_NATIVE_FILE_HPP

#include <nudb/posix_file.hpp>
#include <nudb/win32_file.hpp>
#include <string>

namespace nudb {

/** A native file handle.

    This type is set to the appropriate platform-specific
    implementation to meet the file wrapper requirements.
*/
using native_file =
#ifdef _MSC_VER
    win32_file;
#else
    posix_file;
#endif

} // nudb

#endif
