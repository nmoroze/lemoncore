# Verifying Lemoncore

Verifying Lemoncore with RISC-V Formal (WIP)

Status:
- All instruction checks pass

## Prereqs
1) Symbiyosys
2) Boolector

Ensure riscv-formal submodule is downloaded: `git submodule update --init --recursive`

## Use
`python3 riscv-formal/checks/genchecks.py && make -C checks | grep DONE`

(optionally use `-j$(nproc)` in `make` call for SPEED)
