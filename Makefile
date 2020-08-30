.PHONY: sim clean prog sim-core sim-soc soc test test-core test-soc
.SECONDARY:

all: lemonsoc-timing.rpt lemonsoc-utilization.rpt lemonsoc.bit

## Firmware ##
PREFIX ?= riscv64-unknown-elf-

CC := $(PREFIX)gcc
AS := $(PREFIX)as
LD := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy
OBJDUMP := $(PREFIX)objdump

CFLAGS := -Og -march=rv32i -mabi=ilp32 -fdata-sections -ffunction-sections -ffreestanding
ASFLAGS := -march=rv32i -mabi=ilp32
OBJDUMPFLAGS := --disassemble-all --source --section-headers --demangle
LDFLAGS := -melf32lriscv -nostdlib

LIB_OBJS := sw/lemonlib/entry.o sw/lemonlib/lemonlib.o
LIB_SIM_OBJS := sw/lemonlib/entry.o sw/lemonlib/lemonlib.sim.o

%.sim.o: %.c
	$(CC) $(CFLAGS) -DSIM -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(AS) $(ASFLAGS) -c $< -o $@

%.sim.elf: sw/rom.ld %.sim.o $(ADD_OBJS_SIM)
	$(LD) $(LDFLAGS) -T $< $*.sim.o $(ADD_OBJS_SIM) -o $@

%.elf: sw/rom.ld %.o $(ADD_OBJS)
	$(LD) $(LDFLAGS) -T $< $*.o $(ADD_OBJS) -o $@

%.bin: %.elf
	$(OBJCOPY) $< -O binary $@

%.mem: %.bin
	bin2coe --mem -i $< -o $@ -w 32 --depth 3072 --fill 0

sw/hello.elf: ADD_OBJS = $(LIB_OBJS)
sw/hello.elf: $(LIB_OBJS)
sw/hello.sim.elf: ADD_OBJS_SIM = $(LIB_SIM_OBJS)
sw/hello.sim.elf: $(LIB_SIM_OBJS)

# source lists
# top level module must come first for Verilator recipes to work
CORE_V_SRCS := $(addprefix rtl/core/, lemoncore.v alu.v decoder.v ext.v regfile.v)
CORE_V_INC  := rtl/core/control_signals.vh
SOC_V_SRCS  := $(addprefix rtl/soc/, lemonsoc.v gpio.v ram.v sync.v timer.v) $(CORE_V_SRCS)
SOC_V_INC   := rtl/soc/memmap.vh $(CORE_V_INC)

# fw var for recipes that reference particular firmware
FW ?= hello
FW_PATH = sw/$(FW).mem
SIM_FW_PATH = sw/$(FW).sim.mem

## Verilator simulation ##
CORE_TESTS := sw/tests/test-insertion-sort.s sw/tests/test-exception-handler.s
CORE_TESTS_O = $(patsubst %.s, %.bin, $(CORE_TESTS))
SOC_TESTS_FW := sw/hello.sim.mem

# TODO: compile all tests into one executable
test:
	$(MAKE) test-soc
	$(MAKE) test-core
	$(MAKE) test-decoder
	$(MAKE) test-regfile
	$(MAKE) test-ext
	$(MAKE) test-alu

test-%: obj_dir/%.verilator
	$<

test-core: obj_dir/lemontest.verilator
	$<

test-soc: obj_dir/lemonsoc_tb.verilator $(SIM_FW_PATH)
	$<

sim: sim-soc

sim-core: obj_dir/lemonsim.verilator
	$<

sim-soc: obj_dir/lemonsoc_sim.verilator $(SIM_FW_PATH)
	$<

MODULE_TB_CPP_SRCS := sim/riscv.cpp sim/verilator-gtest-runner.cpp
obj_dir/%.verilator: rtl/core/%.v sim/%_tb.cpp rtl/core/control_signals.vh $(MODULE_TB_CPP_SRCS) sim/riscv.h
	verilator -CFLAGS "-std=gnu++14" -LDFLAGS "-lpthread -lgtest" -Wall -cc $< -Irtl/core --exe \
		--build sim/$*_tb.cpp $(MODULE_TB_CPP_SRCS) -o $(notdir $@)

CORE_TB_CPP_SRCS := sim/lemoncore_tb.cpp sim/lemoncore.cpp sim/util.cpp sim/riscv.cpp sim/verilator-gtest-runner.cpp
obj_dir/lemontest.verilator: $(CORE_V_SRCS) $(CORE_V_INC) $(CORE_TB_CPP_SRCS) $(CORE_TESTS_O) sim/lemoncore.h sim/util.h sim/riscv.h
	verilator -CFLAGS "-std=gnu++14" -LDFLAGS "-lpthread -lgtest" --trace -Wall -cc $< -Irtl/core --exe \
		--build $(CORE_TB_CPP_SRCS) -o $(notdir $@)

CORE_SIM_CPP_SRCS := sim/lemoncore_sim.cpp sim/lemoncore.cpp sim/util.cpp
obj_dir/lemonsim.verilator: $(CORE_V_SRCS) $(CORE_V_INC) $(CORE_SIM_CPP_SRCS) $(FW_PATH) sim/lemoncore.h sim/util.h
	verilator -CFLAGS "-std=gnu++14" --trace -Wall -cc $< -Irtl/core --exe \
		--build $(CORE_SIM_CPP_SRCS) -o $(notdir $@)

SOC_SIM_CPP_SRCS := sim/lemonsoc_sim.cpp sim/lemonsoc.cpp
obj_dir/lemonsoc_sim.verilator: $(SOC_V_SRCS) $(SOC_V_INC) $(SOC_SIM_CPP_SRCS) sim/lemonsoc.h
	verilator -CFLAGS "-std=gnu++14 -DFIRMWARE_PATH='\"$(SIM_FW_PATH)\"'" -DSIM --trace -Wall -LDFLAGS "-lncurses" \
		-cc $< -Irtl/core -Irtl/soc --exe --build  $(SOC_SIM_CPP_SRCS) -o $(notdir $@)

SOC_TB_CPP_SRCS := sim/lemonsoc_tb.cpp sim/lemonsoc.cpp sim/riscv.cpp  sim/verilator-gtest-runner.cpp
obj_dir/lemonsoc_tb.verilator: $(SOC_V_SRCS) $(SOC_V_INC) $(SOC_TB_CPP_SRCS) $(SOC_TESTS_FW) sim/lemonsoc.h sim/riscv.h
	verilator -CFLAGS "-std=gnu++14" -DSIM --trace -Wall -LDFLAGS "-lpthread -lgtest" -cc $< -Irtl/core -Irtl/soc \
		--exe --build $(SOC_TB_CPP_SRCS) -o $(notdir $@)

## FPGA ##
PROJ = lemonsoc

PIN_DEF = icebreaker.pcf
DEVICE = up5k
PACKAGE = sg48
FREQ = 13

random.mem:
	icebram -s 0 -g 32 3072 > $@

lemonsoc.json: $(SOC_V_SRCS) $(SOC_V_INC) random.mem
	yosys -ql $(PROJ)-synth.log  -p 'synth_ice40 -top lemonsoc -json $@' $(SOC_V_SRCS)

$(PROJ).asc: $(PIN_DEF) $(PROJ).json
	nextpnr-ice40 --$(DEVICE) -l $(PROJ)-pnr.log $(if $(PACKAGE),--package $(PACKAGE)) $(if $(FREQ),--freq $(FREQ)) --json $(filter-out $<,$^) --pcf $< --asc $@

$(PROJ)-$(FW).asc: $(PROJ).asc random.mem $(FW_PATH)
	icebram -v random.mem $(FW_PATH) < $< > $@

%.bit: %.asc
	icepack $< $@

%-timing.rpt: %.asc
	icetime $(if $(FREQ),-c $(FREQ)) -d $(DEVICE) -mtr $@ $<

%-utilization.rpt: %.asc
	cat $(PROJ)-pnr.log | grep -A 16 "Info: Device utilisation:" > $@
	icebox_stat $< >> $@

prog: $(PROJ)-$(FW).bit
	iceprog $<

clean:
	rm -f *.asc *.rpt *.bit *.json *.log random.mem
	rm -rf obj_dir/ sim/*.vcd *.vcd
	rm -f sw/*/*.o sw/*/*.elf sw/*/*.bin sw/*/*.mem \
		sw/*.o sw/*.elf sw/*.bin sw/*.mem
