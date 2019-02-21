#
# Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/NuDB
#

set (SOURCE_FILES
    nudb.cpp
)

add_executable (nudb ${SOURCE_FILES})
target_link_libraries(nudb NuDB::nudb NuDB::common)
source_group ("" FILES ${SOURCE_FILES})
common_sources_tree (nudb)
