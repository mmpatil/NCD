#!/bin/sh
 
if [ $(CC) = "gcc" ] ; then
    sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
    sudo apt-get install -qq g++-4.8
    export CXX="g++-4.8" 
fi
    sudo apt-get update -qq
    sudo apt-get install libgtest-dev
    cd /usr/src/gtest && sudo cmake .
    sudo cmake --build .
    sudo mv libg* /usr/local/lib/
    cd -

