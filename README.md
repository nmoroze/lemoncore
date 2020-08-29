# Lemoncore :lemon:

[![Build Status](https://travis-ci.com/nmoroze/lemoncore.svg?branch=main)](https://travis-ci.com/nmoroze/lemoncore)

Lemoncore is a simple [RISC-V][riscv] processor core targeting FPGAs. It
implements the base RV32I instruction set, along with M-mode from the
RISC-V privilege spec.

This repository contains the implementation of Lemoncore itself, along with
a simple SoC implementation, automated tests, simulation code, and example
software.

## Getting Started

The easiest way to get started with Lemoncore is to run the SoC simulation.

The simulation requires installing the following dependencies:

- [RISC-V toolchain][riscv]
- [Verilator (minimum v4.034)][verilator]

Then, run `make sim` to run the simulator on the default firmware (`sw/hello.c`).

See below for more details on using the [simulation](#simulation), as well as
instructions for [running tests](#tests) or [building the SoC](#fpga) for
an [Icebreaker FPGA][icebreaker].

## Usage

### Simulation

#### Dependencies
- [RISC-V toolchain][riscv-gcc]
- [Verilator (minimum v4.034)][verilator]

#### Commands
```
make sim-soc FW=<firmware>
```
Runs SoC simulation, with outputs shown as an ASCII representation of the Icebreaker LEDs. Runs
`sw/hello.c` by default, but setting the optional `FW` variable will cause Make
to build `sw/<firmware>.c` and run that file instead.

```
make sim-core FW=<firmware>
```
Runs simulation of Lemoncore alone. This provides outputs as a trace of memory
reads/writes. The software to run can be selected via `FW` as in the SoC
simulation target.


### Tests

#### Dependencies
- [RISC-V toolchain][riscv-gcc]
- [Verilator (minimum v4.034)][verilator]
- [Google Test][gtest]

#### Commands
```
make test
```
Runs all tests.

```
make test-core
make test-soc
make test-alu
make test-decoder
make test-ext
make test-regfile
```
Runs tests for the Lemoncore, SoC, or individual module (alu, decoder, ext, or regfile), respectively.

### FPGA

#### Dependencies
- [RISC-V toolchain][riscv-gcc]
- [bin2coe][bin2coe]
- [Yosys][yosys]
- [nextpnr-ice40][nextpnr]
- [Project Icestorm][icestorm]

#### Commands
```
make
```
Builds bitstream of SoC targeting the Icebreaker.

```
make prog FW=<firmware>
```
Injects compiled `sw/<firmware>.c` into bitstream, and flashes it onto a connected FPGA using
iceprog.

## Repository Contents
#### `rtl/core/`
RTL for Lemoncore CPU.

#### `rtl/soc/`
RTL for a simple SoC that incorporates Lemoncore, memory, a GPIO peripheral, and
a timer module, targeting the Icebreaker FPGA.

#### `sim/*_tb.cpp`
Automated testbenches for Lemoncore, SoC, and individual modules that make up
the core.

#### `sim/*_sim.cpp`
Simulation harnesses for the SoC and CPU.

#### `sw/`
Example software and a simple library that implements a code entry point and
functions for interfacing with SoC peripherals.

## License
This project is copyright 2020 Noah Moroze, released under the [MIT license][mit].

[riscv]: https://riscv.org/
[icebreaker]: https://1bitsquared.com/products/icebreaker
[verilator]: https://www.veripool.org/projects/verilator/wiki/Installing
[riscv-gcc]: https://github.com/riscv/riscv-gnu-toolchain
[bin2coe]: https://pypi.org/project/bin2coe/
[gtest]: https://github.com/google/googletest
[yosys]: http://yosyshq.net/yosys/download.html
[nextpnr]: https://github.com/YosysHQ/nextpnr
[icestorm]: http://bygone.clairexen.net/icestorm/
[mit]: https://opensource.org/licenses/MIT
