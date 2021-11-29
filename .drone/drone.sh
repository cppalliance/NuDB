#!/bin/bash

# Copyright 2020 Rene Rivera, Sam Darwin
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

set -xe
export TRAVIS_BUILD_DIR=$(pwd)
export DRONE_BUILD_DIR=$(pwd)
export TRAVIS_BRANCH=$DRONE_BRANCH
export TRAVIS_EVENT_TYPE=$DRONE_BUILD_EVENT
export VCS_COMMIT_ID=$DRONE_COMMIT
export GIT_COMMIT=$DRONE_COMMIT
export REPO_NAME=$DRONE_REPO
export USER=$(whoami)
export CC=${CC:-gcc}
export PATH=~/.local/bin:/usr/local/bin:$PATH
export BOOST_ROOT="$TRAVIS_BUILD_DIR/_cache/boost_1_77_0"
export EP_CACHE_ROOT="$TRAVIS_BUILD_DIR/_cache/cmake_ep"
export CMAKE_ROOT="$TRAVIS_BUILD_DIR/_cache/cmake"
export LCOV_ROOT="$TRAVIS_BUILD_DIR/_cache/lcov"
export VALGRIND_ROOT="$TRAVIS_BUILD_DIR/_cache/valgrind"

if [ "$DRONE_JOB_BUILDTYPE" == "boost" ]; then

echo '==================================> BEFORE_INSTALL'

echo "Upgrading cmake"
pip3 install cmake --upgrade
. .drone/before-install.sh

echo '==================================> INSTALL'

if [ "${MATRIX_EVAL}" != "" ] ; then eval "${MATRIX_EVAL}"; fi
scripts/install-cmake.sh 3.14.0
if [ $(uname) = "Darwin" ] ; then export PATH="${CMAKE_ROOT}/CMake.app/Contents/bin:${PATH}"; fi
if [ $(uname) = "Linux" ] ; then export PATH="${CMAKE_ROOT}/bin:${PATH}"; fi
scripts/install-boost.sh
scripts/install-coverage.sh
export PATH="${LCOV_ROOT}/bin:${PATH}"

echo '==================================> BEFORE_SCRIPT'

. $DRONE_BUILD_DIR/.drone/before-script.sh

echo '==================================> SCRIPT'

 scripts/build-and-test.sh

elif [ "$DRONE_JOB_BUILDTYPE" == "5a7d01e40a-dded1b7a0c" ]; then

echo '==================================> BEFORE_INSTALL'

. .drone/before-install.sh

echo '==================================> INSTALL'

if [ "${MATRIX_EVAL}" != "" ] ; then eval "${MATRIX_EVAL}"; fi
cd ..
mkdir tmp && cd tmp
git clone -b 'Release_1_8_15' --depth 1 https://github.com/doxygen/doxygen.git
cd doxygen
cmake -H. -Bbuild -DCMAKE_BUILD_TYPE=Release
cd build
sudo make install
cd ../..
wget -O saxonhe.zip https://sourceforge.net/projects/saxon/files/Saxon-HE/9.9/SaxonHE9-9-1-4J.zip/download
unzip saxonhe.zip
sudo rm /usr/share/java/Saxon-HE.jar
sudo cp saxon9he.jar /usr/share/java/Saxon-HE.jar
cd $TRAVIS_BUILD_DIR
pwd
git submodule update --init doc/docca
cd doc
chmod 755 makeqbk.sh
./makeqbk.sh
cd ..
sed -i 's,path-constant TEST_MAIN : $(BOOST_ROOT)/boost/beast/_experimental/unit_test/main.cpp ;,,' Jamroot
cd ..
BOOST_BRANCH=develop && [ "$TRAVIS_BRANCH" == "master" ] && BOOST_BRANCH=master || true
git clone -b $BOOST_BRANCH https://github.com/boostorg/boost.git boost-root
cd boost-root
export BOOST_ROOT=$(pwd)
git submodule update --init libs/context
git submodule update --init tools/boostbook
git submodule update --init tools/boostdep
git submodule update --init tools/docca
git submodule update --init tools/quickbook
rsync -av $TRAVIS_BUILD_DIR/ libs/nudb
python tools/boostdep/depinst/depinst.py ../tools/quickbook
./bootstrap.sh
./b2 headers

echo '==================================> BEFORE_SCRIPT'

. $DRONE_BUILD_DIR/.drone/before-script.sh

echo '==================================> SCRIPT'

echo "using doxygen ; using boostbook ; using saxonhe ;" > tools/build/src/user-config.jam
./b2 -j3 libs/nudb/doc//boostdoc

fi
