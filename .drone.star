# Use, modification, and distribution are
# subject to the Boost Software License, Version 1.0. (See accompanying
# file LICENSE.txt)
#
# Copyright Rene Rivera 2020.

# For Drone CI we use the Starlark scripting language to reduce duplication.
# As the yaml syntax for Drone CI is rather limited.
#
#
globalenv={'BUILD_SYSTEM': 'cmake', 'BOOST_URL': 'https://dl.bintray.com/boostorg/release/1.69.0/source/boost_1_69_0.tar.bz2'}
linuxglobalimage="cppalliance/droneubuntu1604:1"
windowsglobalimage="cppalliance/dronevs2019"

def main(ctx):
  return [
  linux_cxx("COMMENT=docs Job 0", "g++", packages="docbook docbook-xml docbook-xsl xsltproc libsaxonhe-java default-jre-headless flex bison rsync", buildtype="5a7d01e40a-dded1b7a0c", buildscript="drone", image="cppalliance/droneubuntu2004:1", environment={'COMMENT': 'docs', 'DRONE_JOB_UUID': 'b6589fc6ab'}, globalenv=globalenv),
  linux_cxx("VARIANT=coverage MATRIX_EVAL=CC=gcc-7 && CXX Job 1", "gcc-7", packages="g++-7 software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'coverage', 'MATRIX_EVAL': 'CC=gcc-7 && CXX=g++-7', 'DRONE_JOB_UUID': '356a192b79', 'CODECOV_TOKEN': {'from_secret': 'codecov_token'}, 'COVERALLS_REPO_TOKEN': {'from_secret': 'coveralls_repo_token'}}, globalenv=globalenv),
  # not building  #
  #  osx_cxx("VARIANT=debug Job 2", "g++", packages="", buildtype="boost", buildscript="drone", xcode_version="10.1", environment={'VARIANT': 'debug', 'DRONE_JOB_UUID': 'da4b9237ba'}, globalenv=globalenv),
  # not building  #
  #  osx_cxx("VARIANT=release Job 3", "g++", packages="", buildtype="boost", buildscript="drone", xcode_version="10.1", environment={'VARIANT': 'release', 'DRONE_JOB_UUID': '77de68daec'}, globalenv=globalenv),
  linux_cxx("VARIANT=debug MATRIX_EVAL=CC=gcc-5 && CXX=g+ Job 4", "gcc-5", packages="g++-5 software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'debug', 'MATRIX_EVAL': 'CC=gcc-5 && CXX=g++-5', 'DRONE_JOB_UUID': '1b64538924'}, globalenv=globalenv),
  linux_cxx("VARIANT=debug MATRIX_EVAL=CC=gcc-6 && CXX=g+ Job 5", "gcc-6", packages="g++-6 software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'debug', 'MATRIX_EVAL': 'CC=gcc-6 && CXX=g++-6', 'DRONE_JOB_UUID': 'ac3478d69a'}, globalenv=globalenv),
  linux_cxx("VARIANT=debug MATRIX_EVAL=CC=gcc-7 && CXX=g+ Job 6", "gcc-7", packages="g++-7 software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'debug', 'MATRIX_EVAL': 'CC=gcc-7 && CXX=g++-7', 'DRONE_JOB_UUID': 'c1dfd96eea'}, globalenv=globalenv),
  linux_cxx("VARIANT=debug MATRIX_EVAL=CC=gcc-8 && CXX=g+ Job 7", "gcc-8", packages="g++-8 software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'debug', 'MATRIX_EVAL': 'CC=gcc-8 && CXX=g++-8', 'DRONE_JOB_UUID': '902ba3cda1'}, globalenv=globalenv),
  linux_cxx("VARIANT=release MATRIX_EVAL=CC=gcc-8 && CXX= Job 8", "gcc-8", packages="g++-8 software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'release', 'MATRIX_EVAL': 'CC=gcc-8 && CXX=g++-8', 'DRONE_JOB_UUID': 'fe5dbbcea5'}, globalenv=globalenv),
  # not building  #
  #  linux_cxx("VARIANT=reldeb WITH_VALGRIND=1 MATRIX_EVAL=C Job 9", "gcc-8", packages="g++-8 software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg subversion", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'reldeb', 'WITH_VALGRIND': '1', 'MATRIX_EVAL': 'CC=gcc-8 && CXX=g++-8', 'DRONE_JOB_UUID': '0ade7c2cf9'}, globalenv=globalenv),
  linux_cxx("VARIANT=debug MATRIX_EVAL=CC=clang-6.0 && CX Job 10", "clang-6.0", packages="software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg clang-6.0", llvm_os="xenial", llvm_ver="6.0", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'debug', 'MATRIX_EVAL': 'CC=clang-6.0 && CXX=clang++-6.0', 'DRONE_JOB_UUID': 'b1d5781111'}, globalenv=globalenv),
  linux_cxx("VARIANT=debug MATRIX_EVAL=CC=clang-7 && CXX= Job 11", "clang-7", packages="software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg clang-7", llvm_os="xenial", llvm_ver="7", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'debug', 'MATRIX_EVAL': 'CC=clang-7 && CXX=clang++-7', 'DRONE_JOB_UUID': '17ba079149'}, globalenv=globalenv),
  linux_cxx("VARIANT=debug MATRIX_EVAL=CC=clang-8 && CXX= Job 12", "clang-8", packages="software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg clang-8", llvm_os="xenial", llvm_ver="8", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'debug', 'MATRIX_EVAL': 'CC=clang-8 && CXX=clang++-8', 'DRONE_JOB_UUID': '7b52009b64'}, globalenv=globalenv),
  linux_cxx("VARIANT=usan MATRIX_EVAL=CC=clang-8 && CXX=c Job 13", "clang-8", packages="software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg clang-8 libclang-common-8-dev", llvm_os="xenial", llvm_ver="8", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'usan', 'MATRIX_EVAL': 'CC=clang-8 && CXX=clang++-8', 'UBSAN_OPTIONS': "'print_stacktrace=1'", 'DRONE_JOB_UUID': 'bd307a3ec3'}, globalenv=globalenv),
  linux_cxx("VARIANT=asan MATRIX_EVAL=CC=clang-8 && CXX=c Job 14", "clang-8", packages="software-properties-common wget python-software-properties libstdc++6 binutils-gold gdb make ninja-build ccache python-pip npm libc6-dbg clang-8 libclang-common-8-dev llvm-8", llvm_os="xenial", llvm_ver="8", buildtype="boost", buildscript="drone", image="cppalliance/droneubuntu1604:1", environment={'VARIANT': 'asan', 'MATRIX_EVAL': 'CC=clang-8 && CXX=clang++-8', 'DRONE_EXTRA_PRIVILEGED': 'True', 'DRONE_JOB_UUID': 'fa35e19212'}, globalenv=globalenv, privileged=True),
    ]

# from https://github.com/boostorg/boost-ci
load("@boost_ci//ci/drone/:functions.star", "linux_cxx","windows_cxx","osx_cxx","freebsd_cxx")
