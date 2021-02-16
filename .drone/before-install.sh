#!/bin/bash

# Copyright 2020 Rene Rivera, Sam Darwin
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE.txt or copy at http://boost.org/LICENSE_1_0.txt)

if [ "$DRONE_JOB_UUID" = "356a192b79" ] ; then
    ln -s /usr/bin/nodejs /usr/bin/node
    echo "deb https://deb.nodesource.com/node_6.x xenial main" > /etc/apt/sources.list.d/nodesource.list ;
    echo "deb-src https://deb.nodesource.com/node_6.x xenial main" >> /etc/apt/sources.list.d/nodesource.list;
    curl -s https://deb.nodesource.com/gpgkey/nodesource.gpg.key | sudo apt-key add - ;
    sudo apt-get update ;
    apt-get install -y nodejs;

fi
if [ "$DRONE_JOB_UUID" = "bd307a3ec3" ] ; then
    export UBSAN_SYMBOLIZER_PATH="$(which llvm-symbolizer-8)"

fi

