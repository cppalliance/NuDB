# Part of nudb

GroupSources (include/nudb nudb)
GroupSources (extras/nudb extras)
GroupSources (examples/ "/")

add_executable (crash
    ${NUDB_INCLUDES}
    ${EXTRAS_INCLUDES}
    crash.cpp
)

if (WIN32)
    target_link_libraries (crash ${Boost_LIBRARIES})
else ()
    target_link_libraries (crash ${Boost_LIBRARIES} rt Threads::Threads)
endif ()

add_executable (example
    ${NUDB_INCLUDES}
    ${EXTRAS_INCLUDES}
    example.cpp
)

if (WIN32)
    target_link_libraries (example ${Boost_LIBRARIES})
else ()
    target_link_libraries (example ${Boost_LIBRARIES} rt Threads::Threads)
endif ()

add_executable (recover
    ${NUDB_INCLUDES}
    ${EXTRAS_INCLUDES}
    recover.cpp
)

if (WIN32)
    target_link_libraries (recover ${Boost_LIBRARIES})
else ()
    target_link_libraries (recover ${Boost_LIBRARIES} rt Threads::Threads)
endif ()
