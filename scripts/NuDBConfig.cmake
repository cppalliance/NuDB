include (CMakeFindDependencyMacro)
# need to represent system dependencies of the lib here
#[=========================================================[
  Boost
#]=========================================================]
if ((NOT DEFINED BOOST_ROOT) AND (DEFINED ENV{BOOST_ROOT}))
  set (BOOST_ROOT $ENV{BOOST_ROOT})
endif ()
file (TO_CMAKE_PATH "${BOOST_ROOT}" BOOST_ROOT)
if (static OR APPLE OR MSVC)
  set (Boost_USE_STATIC_LIBS ON)
endif ()
set (Boost_USE_MULTITHREADED ON)
if (static OR MSVC)
  set (Boost_USE_STATIC_RUNTIME ON)
else ()
  set (Boost_USE_STATIC_RUNTIME OFF)
endif ()
find_dependency (Boost 1.69
  COMPONENTS
    filesystem
    system
    thread)

include ("${CMAKE_CURRENT_LIST_DIR}/NuDBTargets.cmake")

