# Verifying Lemoncore

Verifying Lemoncore with RISC-V Formal (WIP)

## Prereqs
1) Symbiyosys
2) Boolector

Ensure riscv-formal submodule is downloaded: `git submodule update --init --recursive`

## Use
`python3 riscv-formal/checks/genchecks.py && make -C checks`
