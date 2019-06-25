//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_FILE_HPP
#define NUDB_FILE_HPP

#include <boost/core/ignore_unused.hpp>
#include <cstddef>
#include <string>

namespace nudb {

/// The type used to hold paths to files
using path_type = std::string;

/** Returns the best guess at the volume's block size.

    @param path A path to a file on the device. The file does
    not need to exist.
*/
inline
std::size_t
block_size(path_type const& path)
{
    boost::ignore_unused(path);
    // A reasonable default for many SSD devices
    return 4096;
}

/** File create and open modes.

    These are used by @ref native_file.
*/
enum class file_mode
{
    /// Open the file for sequential reads
    scan,

    /// Open the file for random reads
    read,

    /// Open the file for random reads and appending writes
    append,

    /// Open the file for random reads and writes
    write
};

// Return boolean indicating if path exists
bool path_exists(path_type const& path);

// Return boolean indicating if path is a directory
bool is_dir(path_type const& path);

// Recursively make the specified dir tree
bool mkdir_p(path_type const& path);

// Append an rel-path to a local filesystem path.
// The returned path is normalized for the platform.
path_type
path_cat(
    path_type const& base,
    path_type const& path);

} // nudb

#include <nudb/impl/file.ipp>

#endif
