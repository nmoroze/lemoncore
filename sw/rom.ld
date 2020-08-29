MEMORY
{
    rom (rx): ORIGIN = 0x0, LENGTH = 8k
    ram (rw): ORIGIN = 0x1000, LENGTH = 16k
}

STACK_SIZE = 512;

SECTIONS
{
    . = 0x0;
    .text : { *(.text) } > rom
    .data : { *(.data) } > ram
    .bss : { *(.bss) } > ram
    .stack (NOLOAD) : {
        _stack_bottom = .;
        . = . + STACK_SIZE;
        _stack_start = .;
    } > ram
}

