#!/bin/bash

# Copyright 2020 Rene Rivera, Sam Darwin
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

if [ "$DRONE_JOB_UUID" = "0ade7c2cf9" ] ; then
    scripts/install-valgrind.sh
    export PATH="${VALGRIND_ROOT}/bin:${PATH}"
fi
if [ "$DRONE_JOB_UUID" = "fa35e19212" ] ; then
    sudo ln -s $(which llvm-symbolizer-8) /usr/bin/llvm-symbolizer
fi

