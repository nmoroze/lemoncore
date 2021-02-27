#!/usr/bin/env bash

python3 riscv-formal/checks/genchecks.py && make -j$(nproc) -C checks | grep DONE
