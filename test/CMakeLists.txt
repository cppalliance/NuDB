# Part of nudb

GroupSources(test "/")
GroupSources(include/nudb nudb)
GroupSources(extras/nudb nudb)
GroupSources(extras/beast/include/beast beast)
GroupSources(extras/beast/extras/beast beast)

add_executable(test-all
    ${NUDB_INCLUDES}
    ${BEAST_INCLUDES}
    ../extras/beast/extras/beast/unit_test/main.cpp
    test_util.hpp
    basic_store.cpp
    callgrind_test.cpp
    create.cpp
    error.cpp
    file.cpp
    native_file.cpp
    posix_file.cpp
    recover.cpp
    recover_test.cpp
    store.cpp
    store_test.cpp
    type_traits.cpp
    verify.cpp
    version.cpp
    visit.cpp
    win32_file.cpp
    xxhasher.cpp
)

if (WIN32)
    target_link_libraries(test-all ${Boost_LIBRARIES})
else ()
    target_link_libraries(test-all ${Boost_LIBRARIES} Threads::Threads)
endif ()
