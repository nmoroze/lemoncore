#!/usr/bin/env bash

cd ~
git clone https://github.com/verilator/verilator

unset VERILATOR_ROOT
cd verilator
git checkout v4.038

autoconf
./configure
make -j$(nproc)
sudo make install
