#!/usr/bin/env bash

cd ~
git clone https://github.com/YosysHQ/nextpnr

cd nextpnr
cmake . -DARCH=ice40 -DBUILD_GUI=OFF
make -j$(nproc)
sudo make install
