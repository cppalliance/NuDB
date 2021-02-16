#!/usr/bin/env bash

set -eu
if [[ $(uname) == "Linux" ]]; then
    # Avoid `spurious errors` caused by ~/.npm permission issues
    # Does it already exist? Who owns? What permissions?
    ls -lah ~/.npm || mkdir ~/.npm
    # Make sure we own it
    chown -Rc ${USER} ~/.npm
    # Install coveralls reporter
    mkdir -p node_modules
    npm install coveralls

    # We use this so we can filter the subtrees from our coverage report
    # pip install --upgrade pip==20.3.4
    # pip install --user https://github.com/codecov/codecov-python/archive/master.zip
    curl -s https://codecov.io/bash -o codecov
    chmod 755 codecov
    mkdir -p ~/.local/bin/
    mv codecov ~/.local/bin

    cd /tmp
    wget https://github.com/linux-test-project/lcov/releases/download/v1.14/lcov-1.14.tar.gz
    tar xfz lcov-1.14.tar.gz
    cd lcov-1.14
    mkdir -p ${LCOV_ROOT}
    make install PREFIX=${LCOV_ROOT}
    cd ..
    rm -r lcov-1.14 lcov-1.14.tar.gz
fi


