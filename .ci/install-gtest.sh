#!/usr/bin/env bash

cd ~
git clone https://github.com/google/googletest

cd googletest
mkdir build
cd build
cmake ..
make
sudo make install
