#
# Copyright (c) 2019 Vinnie Falco (vinnie dot falco at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/NuDB
#

cmake_minimum_required (VERSION 3.11)
set (CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/scripts ${CMAKE_MODULE_PATH})
project (NuDB VERSION 1.1)

if (POLICY CMP0074)
  cmake_policy(SET CMP0074 NEW)
endif ()

get_directory_property(has_parent PARENT_DIRECTORY)
if (has_parent)
  set (is_root_project OFF)
else ()
  set (is_root_project ON)
endif ()

set_property (GLOBAL PROPERTY USE_FOLDERS ON)
set_property (DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT nudb)

if (NOT MSVC)
  set (THREADS_PREFER_PTHREAD_FLAG ON)
  find_package (Threads)
endif ()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES ".*Clang") # both Clang and AppleClang
  set (is_clang TRUE)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set (is_gcc TRUE)
endif ()

option (coverage "enable coverage build" OFF)
option (asan "enable address sanitizer" OFF)
option (usan "enable undefined behavior sanitizer" OFF)
option (werr "treat warnings as errors" OFF)

# hacky legacy handling of options via VARIANT variable
if (NOT DEFINED VARIANT AND DEFINED ENV{VARIANT})
  set (VARIANT $ENV{VARIANT} CACHE STRING "" FORCE)
endif ()
if ("${VARIANT}" STREQUAL "coverage")
  set(coverage ON CACHE BOOL "" FORCE)
elseif ("${VARIANT}" STREQUAL "asan")
  set(asan ON CACHE BOOL "" FORCE)
elseif ("${VARIANT}" STREQUAL "usan")
  set(usan ON CACHE BOOL "" FORCE)
elseif ("${VARIANT}" STREQUAL "debug")
  set (CMAKE_BUILD_TYPE Debug)
elseif ("${VARIANT}" STREQUAL "release")
  set (CMAKE_BUILD_TYPE Release)
elseif ("${VARIANT}" STREQUAL "reldeb")
  set (CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

if (coverage OR usan OR asan)
  set (CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

if (NOT CMAKE_BUILD_TYPE AND NOT is_multiconfig)
  message (STATUS "Build type not specified - defaulting to Release")
  set (CMAKE_BUILD_TYPE Release CACHE STRING "build type" FORCE)
endif ()

#[===================================================================[
  honor a reusable location for EP/FC
#]===================================================================]

get_property (is_multiconfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT DEFINED EP_CACHE_ROOT AND DEFINED ENV{EP_CACHE_ROOT})
  set (EP_CACHE_ROOT $ENV{EP_CACHE_ROOT} CACHE STRING "" FORCE)
endif ()
if (NOT EP_CACHE_ROOT)
  set (EP_CACHE_ROOT ${CMAKE_BINARY_DIR}/vendor CACHE STRING "" FORCE)
endif ()
string (REGEX REPLACE "[ \\/%]+" "_" gen_for_path ${CMAKE_GENERATOR})
string (TOLOWER ${gen_for_path} gen_for_path)
set (ep_cache_path
  "${EP_CACHE_ROOT}/${gen_for_path}/${CMAKE_CXX_COMPILER_ID}_${CMAKE_CXX_COMPILER_VERSION}")
if (NOT is_multiconfig)
  set (ep_cache_path "${ep_cache_path}/${CMAKE_BUILD_TYPE}")
endif ()
file(TO_CMAKE_PATH "${ep_cache_path}" ep_cache_path)
message (STATUS "EP cache path: ${ep_cache_path}")
set (FETCHCONTENT_BASE_DIR ${ep_cache_path} CACHE STRING "" FORCE)

#[===================================================================[
  locate boost
#]===================================================================]

if ((NOT DEFINED BOOST_ROOT) AND (DEFINED ENV{BOOST_ROOT}))
  set (BOOST_ROOT $ENV{BOOST_ROOT})
endif ()
file (TO_CMAKE_PATH "${BOOST_ROOT}" BOOST_ROOT)
set (Boost_USE_STATIC_LIBS ON)
if (MSVC)
  set (Boost_USE_STATIC_RUNTIME ON)
endif ()
set (Boost_USE_MULTITHREADED ON)
set (Boost_NO_SYSTEM_PATHS ON)
find_package (Boost 1.69 REQUIRED COMPONENTS
  filesystem
  program_options
  system
  thread
)

# We also need the beast _experimental files for
# unit tests. boost 1.69 as released doesn't have
# these files, so we do a FetchContent at config time
# to get them.
# THIS CAN BE REMOVED WHEN the official boost version
# found above has these files.
include (FetchContent)
FetchContent_Declare(
  beast_extras_src
  GIT_REPOSITORY https://github.com/boostorg/beast
  GIT_TAG boost-1.70.0.beta1
)
FetchContent_GetProperties(beast_extras_src)
if(NOT beast_extras_src_POPULATED)
  message (STATUS "Pausing to download Beast (for unit tests)...")
  FetchContent_Populate(beast_extras_src)
endif()
file(TO_CMAKE_PATH "${beast_extras_src_SOURCE_DIR}" beast_extras_src_SOURCE_DIR)
add_library (beast_extras INTERFACE)
add_library (NuDB::beast_extras ALIAS beast_extras)
target_include_directories (beast_extras
  INTERFACE
    ${beast_extras_src_SOURCE_DIR}/include)

#[===================================================================[
  include the primary target: nudb interface (header only) lib
#]===================================================================]

add_subdirectory (include/nudb)
# if we are included as a subdir, then we only need the core
# lib definition...exit here
if (NOT is_root_project)
  return ()
endif ()

#[===================================================================[
   define a secondary private target (not exported)
   that encapsulates common macros/link flags used for
   internal targets (tests, benchmarks). These settings
   are candidates for compiler toolchain files in the future
#]===================================================================]

add_library (common INTERFACE)
add_library (NuDB::common ALIAS common)
set_target_properties (common
  PROPERTIES INTERFACE_POSITION_INDEPENDENT_CODE ON)
target_compile_features (common INTERFACE cxx_std_14)
if (MSVC)
  target_compile_definitions (common INTERFACE
    _WIN32_WINNT=0x0601
    _SCL_SECURE_NO_WARNINGS=1
    _CRT_SECURE_NO_WARNINGS=1
    #_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING
    #_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING
  )
  target_compile_options (common INTERFACE
    -bigobj       # large object file format
    -permissive-  # strict C++
    #-wd4503      # decorated name length exceeded, name was truncated
    -W4           # enable all warnings
    -MP           # Multi-processor compilation
    $<$<CONFIG:Debug>:-MTd>
    $<$<NOT:$<CONFIG:Debug>>:-Oi -Ot -MT>
    $<$<CONFIG:Release>:-GL>
    $<$<BOOL:${werr}>:-WX>
  )
  target_link_libraries (common INTERFACE
    -safeseh:no
    $<$<CONFIG:Release>:-ltcg>
    $<$<CONFIG:RelWithDebInfo>:-incremental:no>
  )
else ()
  target_compile_options (common INTERFACE
    -Wall
    -Wextra
    -Wpedantic
    -Wno-unused-parameter
    $<$<BOOL:${is_clang}>:-Wrange-loop-analysis -fvisibility=hidden>
    $<$<BOOL:${werr}>:-Werror>
  )
endif ()

target_link_libraries (common INTERFACE
  Boost::boost
  Boost::filesystem
  Boost::thread
  Boost::system
  Boost::program_options
  Boost::disable_autolinking
  NuDB::beast_extras
)

#[===================================================================[
  interface lib for ubsan
#]===================================================================]

add_library (ubsan_lib INTERFACE)
add_library (NuDB::ubsan ALIAS ubsan_lib)
target_compile_options (ubsan_lib INTERFACE
  -fsanitize=undefined
  -fno-omit-frame-pointer
)
target_link_libraries (ubsan_lib INTERFACE
  -fsanitize=undefined
  $<$<BOOL:${is_gcc}>:ubsan>
)
if (usan)
  target_link_libraries (common INTERFACE NuDB::ubsan)
endif ()

#[===================================================================[
  interface lib for asan
#]===================================================================]

add_library (asan_lib INTERFACE)
add_library (NuDB::asan ALIAS asan_lib)
target_compile_options (asan_lib INTERFACE
  -fsanitize=address
  -fno-omit-frame-pointer
)
target_link_libraries (asan_lib INTERFACE
  -fsanitize=address
  $<$<BOOL:${is_gcc}>:asan>
)
if (asan)
  target_link_libraries (common INTERFACE NuDB::asan)
endif ()

#[===================================================================[
  interface lib for coverage options
#]===================================================================]

add_library (cov_lib INTERFACE)
add_library (NuDB::coverage ALIAS cov_lib)
target_compile_options (cov_lib INTERFACE
  $<$<BOOL:${is_gcc}>:-fprofile-arcs -ftest-coverage>
  $<$<BOOL:${is_clang}>:-fprofile-instr-generate -fcoverage-mapping>
)
target_link_libraries (cov_lib INTERFACE
  $<$<BOOL:${is_gcc}>:-lgcov -fprofile-arcs -ftest-coverage>
  $<$<BOOL:${is_clang}>:-fprofile-instr-generate -fcoverage-mapping>
)
if (coverage)
  target_link_libraries (common INTERFACE NuDB::coverage)
endif ()

#[===================================================================[
  function to help with VS organization
#]===================================================================]

file (GLOB_RECURSE BEAST_FILES
  ${Boost_INCLUDE_DIR}/boost/beast/*.hpp
  ${Boost_INCLUDE_DIR}/boost/beast/*.ipp
)
file(GLOB_RECURSE CORE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/nudb/*.*)

# header files added as "sources" ONLY for convenience so that
# IDEs like VisualStudio will show them
macro(common_sources_tree target)
  target_sources (${target} PRIVATE ${CORE_FILES} ${BEAST_FILES})
  source_group (TREE ${Boost_INCLUDE_DIR}/boost/beast PREFIX "beast" FILES ${BEAST_FILES})
  source_group (TREE ${CMAKE_SOURCE_DIR}/include/nudb PREFIX "nudb" FILES ${CORE_FILES})
endmacro()

#[===================================================================[
   tests/tools
#]===================================================================]

add_subdirectory (bench)
add_subdirectory (examples)
add_subdirectory (test)
add_subdirectory (tools)

