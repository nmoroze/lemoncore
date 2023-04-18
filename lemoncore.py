import siliconcompiler

def setup():
    chip = siliconcompiler.Chip('lemoncore')

    chip.input('rtl/core/alu.v')
    chip.input('rtl/core/decoder.v')
    chip.input('rtl/core/ext.v')
    chip.input('rtl/core/lemoncore.v')
    chip.input('rtl/core/regfile.v')

    chip.add('option', 'idir', 'rtl/core')

    chip.clock('clk_i', period=100)

    return chip

def build():
    chip = setup()

    chip.create_cmdline('Build Lemoncore', switchlist=[
        '-remote',
        '-steplist'
    ])

    chip.load_target('skywater130_demo')
    chip.run()
    chip.summary()

if __name__ == '__main__':
    build()
