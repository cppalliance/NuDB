#
# Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/NuDB
#

set (SOURCE_FILES
    Jamfile
    example.cpp
)

source_group (TREE ${Boost_INCLUDE_DIR}/boost/beast PREFIX beast FILES ${BEAST_FILES})
source_group (TREE ${PROJECT_SOURCE_DIR}/include/nudb PREFIX nudb FILES ${NUDB_FILES})
source_group (TREE ${PROJECT_SOURCE_DIR}/examples PREFIX "" FILES ${SOURCE_FILES})
source_group ("" FILES ${TEST_MAIN})

add_executable (example
    ${NUDB_FILES}
    ${SOURCE_FILES}
)
