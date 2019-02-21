include (ExternalProject)

project (bench)

if (NOT MSVC)
  set (THREADS_PREFER_PTHREAD_FLAG ON)
  find_package (Threads)
endif ()

option (with_rocks "enable rocksdb in benchmark util" ON)
if (WITH_ROCKSDB)
  # legacy flag support
  set(with_rocks ON CACHE BOOL "" FORCE)
endif ()

if (with_rocks)
  #[===================================================================[
      EP dep: rocksdb
  #]===================================================================]

  include (ProcessorCount)
  ProcessorCount(num_procs)
  if (num_procs GREATER 1)
    math (EXPR num_procs "${num_procs} / 2")
  endif ()

  set (ep_lib_prefix ${CMAKE_STATIC_LIBRARY_PREFIX})
  set (ep_lib_suffix ${CMAKE_STATIC_LIBRARY_SUFFIX})

  # snappy - needed by rocks
  ExternalProject_Add (snappy
    PREFIX ${ep_cache_path}
    GIT_REPOSITORY https://github.com/google/snappy.git
    GIT_TAG 1.1.7
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      $<$<BOOL:${CMAKE_VERBOSE_MAKEFILE}>:-DCMAKE_VERBOSE_MAKEFILE=ON>
      -DCMAKE_DEBUG_POSTFIX=_d
      $<$<NOT:$<BOOL:${is_multiconfig}>>:-DCMAKE_BUILD_TYPE=$<IF:$<CONFIG:Debug>,Debug,Release>>
      -DWITH_RUNTIME_DEBUG=$<IF:$<CONFIG:Debug>,ON,OFF>
      -DBUILD_SHARED_LIBS=OFF
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      -DSNAPPY_BUILD_TESTS=OFF
      $<$<BOOL:${MSVC}>:
        "-DCMAKE_CXX_FLAGS=-GR -Gd -fp:precise -FS -MP"
        "-DCMAKE_CXX_FLAGS_DEBUG=-MTd -GS"
        "-DCMAKE_CXX_FLAGS_RELEASE=-MT -Ox"
      >
    LOG_BUILD ON
    LOG_CONFIGURE ON
    BUILD_COMMAND
      ${CMAKE_COMMAND}
      --build .
      --config $<CONFIG>
      $<$<VERSION_GREATER_EQUAL:${CMAKE_VERSION},3.12>:--parallel ${num_procs}>
      $<$<BOOL:${is_multiconfig}>:
        COMMAND
          ${CMAKE_COMMAND} -E copy
          <BINARY_DIR>/$<CONFIG>/${ep_lib_prefix}snappy$<$<CONFIG:Debug>:_d>${ep_lib_suffix}
          <BINARY_DIR>
      >
    TEST_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_BYPRODUCTS
      <BINARY_DIR>/${ep_lib_prefix}snappy${ep_lib_suffix}
      <BINARY_DIR>/${ep_lib_prefix}snappy_d${ep_lib_suffix}
  )
  ExternalProject_Get_Property (snappy BINARY_DIR)
  ExternalProject_Get_Property (snappy SOURCE_DIR)
  add_library (snappy_lib STATIC IMPORTED GLOBAL)
  file (MAKE_DIRECTORY ${SOURCE_DIR}/snappy)
  set_target_properties (snappy_lib PROPERTIES
    IMPORTED_LOCATION_DEBUG
      ${BINARY_DIR}/${ep_lib_prefix}snappy_d${ep_lib_suffix}
    IMPORTED_LOCATION
      ${BINARY_DIR}/${ep_lib_prefix}snappy${ep_lib_suffix}
    INTERFACE_INCLUDE_DIRECTORIES
      "${SOURCE_DIR};${BINARY_DIR}")
  add_dependencies (snappy_lib snappy)

  # rocksdb

  ExternalProject_Add (rocksdb
    PREFIX ${ep_cache_path}
    GIT_REPOSITORY https://github.com/facebook/rocksdb.git
    GIT_TAG v5.17.2
    PATCH_COMMAND
      # only used by windows build
      ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb_support/rocks_thirdparty.inc
      <SOURCE_DIR>/thirdparty.inc
    COMMAND
      # fixup their build version file to keep the values
      # from changing always
      ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/rocksdb_support/rocksdb_build_version.cc.in
      <SOURCE_DIR>/util/build_version.cc.in
    CMAKE_ARGS
      -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
      -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
      $<$<BOOL:${CMAKE_VERBOSE_MAKEFILE}>:-DCMAKE_VERBOSE_MAKEFILE=ON>
      -DCMAKE_DEBUG_POSTFIX=_d
      $<$<NOT:$<BOOL:${is_multiconfig}>>:-DCMAKE_BUILD_TYPE=$<IF:$<CONFIG:Debug>,Debug,Release>>
      -DBUILD_SHARED_LIBS=OFF
      -DCMAKE_POSITION_INDEPENDENT_CODE=ON
      -DWITH_JEMALLOC=OFF
      -DWITH_SNAPPY=ON
      -USNAPPY_*
      -DSNAPPY_INCLUDE_DIR=$<JOIN:$<TARGET_PROPERTY:snappy_lib,INTERFACE_INCLUDE_DIRECTORIES>,::>
      -DSNAPPY_LIBRARIES=$<IF:$<CONFIG:Debug>,$<TARGET_PROPERTY:snappy_lib,IMPORTED_LOCATION_DEBUG>,$<TARGET_PROPERTY:snappy_lib,IMPORTED_LOCATION>>
      -DSNAPPY_FOUND=ON
      -DWITH_LZ4=OFF
      -DWITH_ZLIB=$<IF:$<BOOL:${MSVC}>,OFF,ON>
      -DUSE_RTTI=ON
      -DWITH_ZSTD=OFF
      -DWITH_GFLAGS=OFF
      -DWITH_BZ2=OFF
      -DWITH_MD_LIBRARY=OFF
      -DWITH_RUNTIME_DEBUG=$<IF:$<CONFIG:Debug>,ON,OFF>
      -DFAIL_ON_WARNINGS=OFF
      -DWITH_ASAN=OFF
      -DWITH_TSAN=OFF
      -DWITH_UBSAN=OFF
      -DWITH_NUMA=OFF
      -DWITH_TBB=OFF
      -DWITH_WINDOWS_UTF8_FILENAMES=OFF
      -DWITH_XPRESS=OFF
      -DPORTABLE=OFF
      -DFORCE_SSE42=OFF
      -DDISABLE_STALL_NOTIF=OFF
      -DOPTDBG=ON
      -DROCKSDB_LITE=OFF
      -DWITH_FALLOCATE=ON
      -DWITH_LIBRADOS=OFF
      -DWITH_JNI=OFF
      -DROCKSDB_INSTALL_ON_WINDOWS=OFF
      -DWITH_TESTS=OFF
      -DWITH_TOOLS=OFF
      $<$<BOOL:${MSVC}>:
        "-DCMAKE_CXX_FLAGS=-GR -Gd -fp:precise -FS -MP /DNDEBUG"
      >
      $<$<NOT:$<BOOL:${MSVC}>>:
        "-DCMAKE_CXX_FLAGS=-DNDEBUG"
      >
    LOG_BUILD ON
    LOG_CONFIGURE ON
    BUILD_COMMAND
      ${CMAKE_COMMAND}
      --build .
      --config $<CONFIG>
      $<$<VERSION_GREATER_EQUAL:${CMAKE_VERSION},3.12>:--parallel ${num_procs}>
      $<$<BOOL:${is_multiconfig}>:
        COMMAND
          ${CMAKE_COMMAND} -E copy
          <BINARY_DIR>/$<CONFIG>/${ep_lib_prefix}rocksdb$<$<CONFIG:Debug>:_d>${ep_lib_suffix}
          <BINARY_DIR>
        >
    LIST_SEPARATOR ::
    TEST_COMMAND ""
    INSTALL_COMMAND ""
    DEPENDS snappy
    BUILD_BYPRODUCTS
      <BINARY_DIR>/${ep_lib_prefix}rocksdb${ep_lib_suffix}
      <BINARY_DIR>/${ep_lib_prefix}rocksdb_d${ep_lib_suffix}
  )
  ExternalProject_Get_Property (rocksdb BINARY_DIR)
  ExternalProject_Get_Property (rocksdb SOURCE_DIR)
  add_library (rocksdb_lib STATIC IMPORTED GLOBAL)
  file (MAKE_DIRECTORY ${SOURCE_DIR}/include)
  set_target_properties (rocksdb_lib PROPERTIES
    IMPORTED_LOCATION_DEBUG
      ${BINARY_DIR}/${ep_lib_prefix}rocksdb_d${ep_lib_suffix}
    IMPORTED_LOCATION
      ${BINARY_DIR}/${ep_lib_prefix}rocksdb${ep_lib_suffix}
    INTERFACE_INCLUDE_DIRECTORIES
      ${SOURCE_DIR}/include
    INTERFACE_COMPILE_DEFINITIONS
      WITH_ROCKSDB)
  add_dependencies (rocksdb_lib rocksdb)
  target_link_libraries (rocksdb_lib INTERFACE
    snappy_lib
    $<$<BOOL:${MSVC}>:rpcrt4 shlwapi>
    $<$<NOT:$<BOOL:${MSVC}>>:z>
  )
endif ()

#[===================================================================[
    bench executable
#]===================================================================]

add_executable(bench bench.cpp)
target_link_libraries(bench NuDB::nudb NuDB::common)
if (with_rocks)
  target_link_libraries(bench rocksdb_lib)
endif ()
if(MSVC)
  target_compile_options (bench PRIVATE
    -bigobj            # Increase object file max size
    -fp:precise        # Floating point behavior
    -Gd                # __cdecl calling convention
    -Gm-               # Minimal rebuild: disabled
    -Gy-               # Function level linking: disabled
    -MP                # Multiprocessor compilation
    -openmp-           # pragma omp: disabled
    -errorReport:none  # No error reporting to Internet
    -nologo            # Suppress login banner
    -wd4018            # Disable signed/unsigned comparison warnings
    -wd4244            # Disable float to int possible loss of data warnings
    -wd4267            # Disable size_t to T possible loss of data warnings
    -wd4800            # Disable C4800(int to bool performance)
    -wd4503            # Decorated name length exceeded, name was truncated
    -W3
    $<$<BOOL:${werr}>:-WX>
    -EHa
    -GR
    $<$<CONFIG:Release>:-Ox>
    $<$<CONFIG:Debug>:-MTd -GS -Zc:forScope>
    # static runtime
    $<$<NOT:$<CONFIG:Debug>>:-MT>
  )
  target_compile_definitions (bench PRIVATE
    _WIN32_WINNT=0x6000
    _SCL_SECURE_NO_WARNINGS
    _CRT_SECURE_NO_WARNINGS
    WIN32_CONSOLE
    NOMINMAX
    $<$<CONFIG:Debug>:_CRTDBG_MAP_ALLOC>
  )
  target_link_options (bench PRIVATE
    -errorreport:none
    -machine:X64
  )
else ()
  target_link_libraries(bench
    dl
    Threads::Threads
    $<$<NOT:$<BOOL:${APPLE}>>:rt>)
endif ()
source_group ("" FILES bench.cpp)
common_sources_tree (bench)
