#
# Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/NuDB
#

set (SOURCE_FILES
    example.cpp
)

add_executable (example ${SOURCE_FILES})
target_link_libraries(example NuDB::nudb NuDB::common)
source_group ("" FILES ${SOURCE_FILES})
common_sources_tree (example)

