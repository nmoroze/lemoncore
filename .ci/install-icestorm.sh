#!/usr/bin/env bash

cd ~
git clone https://github.com/YosysHQ/icestorm

cd icestorm
make -j$(nproc)
sudo make install
