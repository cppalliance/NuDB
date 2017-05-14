# Part of nudb

GroupSources (include/nudb nudb)
GroupSources (extras/nudb extras)
GroupSources (examples/ "/")

add_executable (example
    ${NUDB_INCLUDES}
    ${EXTRAS_INCLUDES}
    example.cpp
)
add_executable (crash
    ${NUDB_INCLUDES}
    ${EXTRAS_INCLUDES}
    crash.cpp
)
add_executable (recover
    ${NUDB_INCLUDES}
    ${EXTRAS_INCLUDES}
    reconver.cpp
)

if (WIN32)
    target_link_libraries (example ${Boost_LIBRARIES})
else ()
    target_link_libraries (example ${Boost_LIBRARIES} rt Threads::Threads)
endif ()
