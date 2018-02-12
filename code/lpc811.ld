/*
 * generic gcc linker script for LPC810
 * Kamal Mostafa <kamal@whence.com>.
 * Modified for LPC811 by Matt Sarnoff <matt.sarnoff@gmail.com>
 *
 * License: Public Domain
 */

MEMORY
{
    /* NXP LPC811 */
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 0x00002000 /* 8 Kbytes */
    RAM (rwx) :  ORIGIN = 0x10000000, LENGTH = 0x00000800 /* 2 Kbytes */
}

SECTIONS
{
    .text :
    {
        _text = .;
        KEEP(*(.isr_vector))
        KEEP(*(.after_vectors))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH

    .data : AT (_etext)
    {
        _data = .;
        *(vtable)
        *(.data*)
        _edata = .;
    } > RAM

    .bss :
    {
        _bss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM
}
