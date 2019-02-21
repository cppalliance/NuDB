#!/usr/bin/env bash
# Assumptions:
# 1) BOOST_ROOT and BOOST_URL are already defined,
# and contain valid values.
# 2) The last namepart of BOOST_ROOT matches the
# folder name internal to boost's .tar.gz
# When testing you can force a boost build by clearing travis caches:
# https://travis-ci.org/vinniefalco/nudb/caches
set -eu

if [[ -d "$BOOST_ROOT/lib" || -d "${BOOST_ROOT}/stage/lib" ]] ; then
  echo "Using cached boost at $BOOST_ROOT"
  exit
fi

fn=$(basename -- "$BOOST_URL")
ext="${fn##*.}"
wget --quiet $BOOST_URL -O /tmp/boost.tar.${ext}
cd $(dirname $BOOST_ROOT)
rm -fr ${BOOST_ROOT}
tar xf /tmp/boost.tar.${ext}
cd $BOOST_ROOT

if [[ -z ${COMSPEC:-} ]]; then
  params="cxxflags=\"-std=c++14\" \
    --with-program_options --with-system --with-date_time \
    --with-coroutine --with-filesystem --with-atomic"
  ./bootstrap.sh --prefix=$BOOST_ROOT
  eval ./b2 -d1 -j2 $params
  eval ./b2 -d0 -j2 $params install
else
  cmd /E:ON /D /S /C"bootstrap.bat"
  ./b2.exe -j2 \
    --toolset=msvc-14.1 \
    address-model=64 \
    architecture=x86 \
    link=static \
    threading=multi \
    runtime-link="shared,static" \
    --with-program_options \
    --with-system \
    --with-date_time \
    --with-coroutine \
    --with-filesystem \
    --with-atomic \
    stage
fi

