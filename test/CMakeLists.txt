#
# Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/NuDB
#

set (TEST_MAIN ${Boost_INCLUDE_DIR}/boost/beast/_experimental/unit_test/main.cpp)

set (SOURCE_FILES
    Jamfile
    basic_store.cpp
    buffer.cpp
    callgrind_test.cpp
    concepts.cpp
    create.cpp
    error.cpp
    file.cpp
    native_file.cpp
    posix_file.cpp
    recover.cpp
    rekey.cpp
    store.cpp
    type_traits.cpp
    verify.cpp
    version.cpp
    visit.cpp
    win32_file.cpp
    xxhasher.cpp
)

source_group (TREE ${Boost_INCLUDE_DIR}/boost/beast PREFIX beast FILES ${BEAST_FILES})
source_group (TREE ${PROJECT_SOURCE_DIR}/include/nudb PREFIX nudb FILES ${NUDB_FILES})
source_group (TREE ${PROJECT_SOURCE_DIR}/test PREFIX "" FILES ${SOURCE_FILES})
source_group ("" FILES ${TEST_MAIN})

add_executable(test-all
    ${BEAST_FILES}
    ${TEST_MAIN}
    ${NUDB_FILES}
    ${SOURCE_FILES}
)
