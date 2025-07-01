SECTIONS
{
    . = 0x7C00;
    
    .text : {
        *(.text)
    }
    
    .data : {
        *(.data)
    }
    
    .bss : {
        *(.bss)
    }
    
    /* Boot signature */
    . = 0x7DFE;
    .signature : {
        SHORT(0xAA55)
    }
}
