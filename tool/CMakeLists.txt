# Part of nudb

GroupSources (include/nudb nudb)
GroupSources (extras/nudb nudb)
GroupSources (tool "/")

add_executable (nudb
    ${NUDB_INCLUDES}
    nudb.cpp
    basic_seconds_clock.hpp
    util.hpp
)

if (WIN32)
    target_link_libraries (nudb ${Boost_LIBRARIES})
else ()
    target_link_libraries (nudb ${Boost_LIBRARIES} rt Threads::Threads)
endif ()
