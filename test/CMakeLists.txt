#
# Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/NuDB
#

# FIXME: this _experimental path is not part of official boost beast yet.
# Determine this path from one of the boost targets when it's official,
# but for now get from our local FetchContent
get_target_property(beast_extras_include beast_extras INTERFACE_INCLUDE_DIRECTORIES)
set (TEST_MAIN ${beast_extras_include}/boost/beast/_experimental/unit_test/main.cpp)

set (SOURCE_FILES
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

add_executable(test-all ${SOURCE_FILES} ${TEST_MAIN})
target_link_libraries(test-all NuDB::nudb NuDB::common)
source_group ("" FILES ${TEST_MAIN} ${SOURCE_FILES})
common_sources_tree (test-all)


