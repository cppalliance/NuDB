//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_NATIVE_FILE_HPP
#define NUDB_NATIVE_FILE_HPP


#if ! GENERATING_DOCS

#ifdef _MSC_VER
#include <nudb/win32_file.hpp>
#else
#include <nudb/posix_file.hpp>
#endif

#endif //GENERATING_DOCS


namespace nudb {

/** A native file handle.
This class provides an implementation of the @b File concept for the native file system.
*/
#ifndef _MSC_VER
using native_file = win32_file;
#else
using native_file = posix_file;
#endif

} // nudb

#endif
