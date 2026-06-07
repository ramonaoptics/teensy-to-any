/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ramona Optics, Inc.
 *
 * Clean-room linker script for the Teensy 4.0 (NXP i.MX RT1062, Cortex-M7).
 * Memory addresses/sizes are hardware facts from the i.MX RT1062 reference
 * manual and the Teensy 4.0 board (1984 KiB program flash on the external
 * QSPI part, 512 KiB FlexRAM split into ITCM/DTCM, 512 KiB OCRAM).
 *
 * The image is execute-in-place from FlexSPI flash at 0x60000000:
 *   0x60000000  FlexSPI NOR configuration block (512 bytes)
 *   0x60001000  Image Vector Table (IVT)
 *   0x60001020  Boot Data
 *   0x60002000  program image (vectors, code copied to ITCM, data to DTCM)
 */

MEMORY
{
    ITCM  (rwx) : ORIGIN = 0x00000000, LENGTH = 512K
    DTCM  (rwx) : ORIGIN = 0x20000000, LENGTH = 512K
    OCRAM (rwx) : ORIGIN = 0x20200000, LENGTH = 512K
    FLASH (rx)  : ORIGIN = 0x60000000, LENGTH = 1984K
}

ENTRY(ResetHandler)

SECTIONS
{
    /* --- Boot headers at the base of flash ----------------------------- */
    .boot_hdr ORIGIN(FLASH) :
    {
        KEEP(*(.flexspi_config))            /* 0x60000000, 512 bytes        */
        . = ORIGIN(FLASH) + 0x1000;
        KEEP(*(.ivt))                       /* 0x60001000                   */
        KEEP(*(.boot_data))                 /* 0x60001020                   */
        . = ORIGIN(FLASH) + 0x2000;
    } > FLASH

    /* --- Vector table + code, executed from ITCM ----------------------- */
    .text.itcm :
    {
        . = ALIGN(4);
        KEEP(*(.vectors))
        *(.text*)
        *(.gnu.linkonce.t.*)
        . = ALIGN(16);
    } > ITCM AT> FLASH

    .ARM.exidx :
    {
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } > ITCM AT> FLASH

    _stext = ADDR(.text.itcm);
    _etext = _stext + SIZEOF(.text.itcm) + SIZEOF(.ARM.exidx);
    _flashstart_text = LOADADDR(.text.itcm);

    /* --- Read-only data stays in flash (XIP) --------------------------- */
    .rodata :
    {
        . = ALIGN(4);
        *(.rodata*)
        *(.gnu.linkonce.r.*)
        . = ALIGN(4);
        /* C++ static constructors */
        __init_array_start = .;
        KEEP(*(.init_array*))
        __init_array_end = .;
    } > FLASH

    /* --- Initialised data, runs from DTCM, loaded from flash ----------- */
    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data*)
        *(.gnu.linkonce.d.*)
        . = ALIGN(4);
        _edata = .;
    } > DTCM AT> FLASH
    _flashstart_data = LOADADDR(.data);

    /* --- Zero-initialised data ----------------------------------------- */
    .bss (NOLOAD) :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > DTCM

    /* --- DMA / OCRAM scratch (uninitialised) --------------------------- */
    .ocram (NOLOAD) :
    {
        . = ALIGN(32);
        *(.dmabuffers)
        . = ALIGN(32);
    } > OCRAM

    /* Heap grows up through the remaining OCRAM; stack at top of DTCM. */
    _heap_start = .;
    _heap_end   = ORIGIN(OCRAM) + LENGTH(OCRAM);
    _estack     = ORIGIN(DTCM) + LENGTH(DTCM);

    _flashimagelen = _flashstart_data + SIZEOF(.data) - ORIGIN(FLASH);

    /DISCARD/ : { *(.ARM.attributes) }
}
