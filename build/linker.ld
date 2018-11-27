ENTRY(_start)
 
SECTIONS
{
    /* Starts at LOADER_ADDR. */
    . = 0x8000;
    __start = .;
    __text_start = .;
    .text :
    {
        KEEP(*(.text.boot))
        *(.text)

        _etext = .;
    }
    . = ALIGN(4096); /* align to page size */
    __text_end = .;
 
    __rodata_start = .;
    .rodata :
    {
        *(.rodata)
    }

    .init_array : {
        __init_start = .;

        *(.init_array*)

        __init_end = .;
    }

    .ARM.exidx : {
        __exidx_start = .;

        *(.ARM.exidx*)

        __exidx_end = .;
    }
    
    . = ALIGN(4096); /* align to page size */
    __rodata_end = .;
 
    __data_start = .;
    .data :
    {
        *(.data)
    }
    . = ALIGN(4096); /* align to page size */
    __data_end = .;
 
    __bss_start = .;
    .bss :
    {
        bss = .;
        *(.bss)

        _end = .;
    }
    . = ALIGN(4096); /* align to page size */
    __bss_end = .;
    __end = .;
}
