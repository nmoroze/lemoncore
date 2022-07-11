import os
import subprocess

import pytest

import siliconcompiler

def setup():
    chip = siliconcompiler.Chip('lemoncore')

    chip.add('input', 'verilog', 'rtl/core/alu.v')
    chip.add('input', 'verilog', 'rtl/core/decoder.v')
    chip.add('input', 'verilog', 'rtl/core/ext.v')
    chip.add('input', 'verilog', 'rtl/core/lemoncore.v')
    chip.add('input', 'verilog', 'rtl/core/regfile.v')

    chip.set('option', 'idir', 'rtl/core')

    return chip

def verilator_flow(chip):
    flow = 'verilator_sim'
    chip.node(flow, 'import', 'surelog')
    chip.node(flow, 'compile', 'verilator')
    chip.edge(flow, 'import', 'compile')
    chip.set('option', 'flow', flow)

    idir = os.path.abspath('sim/')
    verilator_flags = [
        '-CFLAGS', f'"-std=gnu++14 -I{idir}"',
        '-LDFLAGS', '"-lpthread -lgtest"',
    ]
    chip.set('tool', 'verilator', 'option', 'compile', '0', verilator_flags)

@pytest.fixture
def tbchip():
    chip = setup()

    verilator_flow(chip)

    chip.add('input', 'c', 'sim/riscv.cpp')
    chip.add('input', 'c', 'sim/verilator-gtest-runner.cpp')

    return chip

@pytest.mark.parametrize('module', ['alu', 'decoder', 'ext', 'lemoncore', 'regfile'])
def test_module(module, tbchip):
    tbchip.set('option', 'entrypoint', module)
    tbchip.set('option', 'jobname', f'{module}_test')

    tbchip.add('input', 'c', f'sim/{module}_tb.cpp')
    if module == 'lemoncore':
        tbchip.set('option', 'trace', True)
        tbchip.add('input', 'c', 'sim/lemoncore.cpp')
        tbchip.add('input', 'c', 'sim/util.cpp')

    tbchip.run()

    result = tbchip.find_result('vexe', step='compile')
    assert result is not None

    proc = subprocess.run([result])
    assert proc.returncode == 0

def simulate(firmware):
    chip = setup()

    chip.add('input', 'c', 'sim/lemoncore_sim.cpp')

    chip.set('option', 'trace', True)
    chip.add('input', 'c', 'sim/lemoncore.cpp')
    chip.add('input', 'c', 'sim/util.cpp')

    verilator_flow(chip)

    chip.run()

    result = chip.find_result('vexe', step='compile')
    subprocess.run([result, f'+firmware+{firmware}'])

if __name__ == '__main__':
    simulate('sw/hello.sim.bin')
