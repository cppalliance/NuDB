//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_FILE_HPP
#define NUDB_FILE_HPP

#include <cstddef>
#include <string>

namespace nudb {

using path_type = std::string;

/** Returns the best guess at the volume's block size. */
inline
std::size_t
block_size(path_type const&)
{
    // A reasonable default for almost all SSD systems
    return 4096;
}

// Commonly used types

enum class file_mode
{
    scan,         // read sequential
    read,         // read random
    append,       // read random, write append
    write         // read random, write random
};

} // nudb

#endif
