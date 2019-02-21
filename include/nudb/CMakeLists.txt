
#[===================================================================[
   define the primary target: nudb (header only lib)
#]===================================================================]

add_library (nudb_core INTERFACE)
target_include_directories (nudb_core
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<INSTALL_INTERFACE:include>)
target_link_libraries (nudb_core
  INTERFACE
    Boost::thread
    Boost::system)
# this is the min std required to use NuDB - should
# it be 11 or 14?
target_compile_features (nudb_core INTERFACE cxx_std_11)
add_library (NuDB::nudb ALIAS nudb_core)
install (
  FILES
    basic_store.hpp
    concepts.hpp
    create.hpp
    error.hpp
    file.hpp
    native_file.hpp
    nudb.hpp
    posix_file.hpp
    progress.hpp
    recover.hpp
    rekey.hpp
    store.hpp
    type_traits.hpp
    verify.hpp
    version.hpp
    visit.hpp
    win32_file.hpp
    xxhasher.hpp
  DESTINATION include/nudb)
install (
  FILES
    _experimental/basic_seconds_clock.hpp
    _experimental/chrono_util.hpp
    _experimental/util.hpp
  DESTINATION include/nudb/_experimental)
# not sure if these should be part of the install set
# are they needed by any public headers ?
install (
  FILES
    _experimental/test/fail_file.hpp
    _experimental/test/temp_dir.hpp
    _experimental/test/test_store.hpp
    _experimental/test/xor_shift_engine.hpp
  DESTINATION include/nudb/_experimental/test)
install (
  FILES
    impl/basic_store.ipp
    impl/create.ipp
    impl/error.ipp
    impl/posix_file.ipp
    impl/recover.ipp
    impl/rekey.ipp
    impl/verify.ipp
    impl/visit.ipp
    impl/win32_file.ipp
  DESTINATION include/nudb/impl)
install (
  FILES
    detail/arena.hpp
    detail/bucket.hpp
    detail/buffer.hpp
    detail/bulkio.hpp
    detail/cache.hpp
    detail/endian.hpp
    detail/field.hpp
    detail/format.hpp
    detail/gentex.hpp
    detail/mutex.hpp
    detail/pool.hpp
    detail/stream.hpp
    detail/xxhash.hpp
  DESTINATION include/nudb/impl)
install (
  TARGETS nudb_core
  EXPORT NuDBExports
  INCLUDES DESTINATION include)
install (EXPORT NuDBExports
  FILE NuDBTargets.cmake
  NAMESPACE NuDB::
  DESTINATION lib/cmake/nudb)
include (CMakePackageConfigHelpers)
write_basic_package_version_file (
  NuDBConfigVersion.cmake
  VERSION ${CMAKE_PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

if (is_root_project)
  install (
    FILES
      ${CMAKE_SOURCE_DIR}/scripts/NuDBConfig.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/NuDBConfigVersion.cmake
    DESTINATION lib/cmake/nudb)
endif ()

