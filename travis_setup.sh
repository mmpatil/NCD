#!/bin/sh

set -e

if [ "$CC" = "gcc" ] ; then
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    sudo apt-get update -qq
    sudo apt-get install -qq gcc-4.8-multilib g++-4.8-multilib
    export CXX="g++-4.8"
    export CC="gcc-4.8"
else
    sudo apt-get update -qq
fi

    sudo apt-get install libgtest-dev:i386 -y
    cd /usr/src/gtest && sudo cmake .
    sudo cmake --build .
    sudo mv libg* /usr/local/lib/
    cd -
