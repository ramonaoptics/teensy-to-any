// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Ramona Optics, Inc.
//
// Clean-room minimal boot/startup for the Teensy 4.0 (i.MX RT1062, Cortex-M7).
//
// Responsibilities:
//   1. Provide the three boot structures the i.MX RT boot ROM reads from flash:
//      the FlexSPI NOR configuration block, the Image Vector Table (IVT), and
//      the Boot Data. Their offsets are fixed by the linker script.
//   2. Provide the Cortex-M7 vector table.
//   3. ResetHandler: copy .text -> ITCM and .data -> DTCM, zero .bss, run C++
//      constructors, and call main().
//
// NOTE: the FlexSPI configuration block contents and the FlexRAM/clock setup
// are hardware-critical and are to be validated on a real Teensy 4.0 (see
// docs/migration). This file is written to LINK and to be the correct *shape*;
// exact timing values are refined during on-hardware bring-up.

#include <stdint.h>
#include "MIMXRT1062.h"   // NXP CMSIS device registers (vendored, BSD-3)

extern int main(void);
extern void __libc_init_array(void);
extern void SystemInit(void);   // NXP system_MIMXRT1062.c

// Linker-provided symbols
extern uint32_t _stext, _etext, _flashstart_text;
extern uint32_t _sdata, _edata, _flashstart_data;
extern uint32_t _sbss, _ebss;
extern uint32_t _estack;

// ---------------------------------------------------------------------------
// 1. FlexSPI NOR configuration block @ 0x60000000 (512 bytes)
// ---------------------------------------------------------------------------
// The boot ROM parses this to configure the external QSPI flash before
// fetching the rest of the image. Tag = "FCFB" (0x42464346).
__attribute__((section(".flexspi_config"), used))
const uint8_t flexspi_config[512] = {
    // Tag 'FCFB', version bytes
    0x46, 0x43, 0x46, 0x42, 0x00, 0x00, 0x01, 0x56,
    // readSampleClkSrc, CSHoldTime, CSSetupTime, columnAddressWidth ...
    // (Remaining lookup-table / timing fields are filled during hardware
    //  bring-up against the Teensy 4.0 flash part. Zero-filled placeholder.)
};

// ---------------------------------------------------------------------------
// 2. Image Vector Table @ 0x60001000 and 3. Boot Data @ 0x60001020
// ---------------------------------------------------------------------------
typedef struct {
    uint32_t header;       // 0x402000D1
    uint32_t entry;        // pointer to ResetHandler (via vector table)
    uint32_t reserved1;
    uint32_t dcd;          // device config data (unused -> 0)
    uint32_t boot_data;    // pointer to boot_data struct
    uint32_t self;         // pointer to this IVT
    uint32_t csf;          // command sequence file (unsigned image -> 0)
    uint32_t reserved2;
} ivt_t;

typedef struct {
    uint32_t start;        // image start (flash origin)
    uint32_t length;       // image length in bytes
    uint32_t plugin;       // 0 = normal boot
} boot_data_t;

extern const ivt_t image_vector_table;
extern const boot_data_t boot_data;
extern uint32_t _flashimagelen;
extern void (*const _vectors[])(void);

__attribute__((section(".boot_data"), used))
const boot_data_t boot_data = {
    .start  = 0x60000000,
    .length = 1984u * 1024u,
    .plugin = 0,
};

__attribute__((section(".ivt"), used))
const ivt_t image_vector_table = {
    .header    = 0x402000D1,
    .entry     = (uint32_t)&_vectors,
    .reserved1 = 0,
    .dcd       = 0,
    .boot_data = (uint32_t)&boot_data,
    .self      = (uint32_t)&image_vector_table,
    .csf       = 0,
    .reserved2 = 0,
};

// ---------------------------------------------------------------------------
// 2b. Cortex-M7 vector table
// ---------------------------------------------------------------------------
void ResetHandler(void);
static void DefaultHandler(void) { while (1) { } }

void NMI_Handler(void)        __attribute__((weak, alias("DefaultHandler")));
void HardFault_Handler(void)  __attribute__((weak, alias("DefaultHandler")));
void MemManage_Handler(void)  __attribute__((weak, alias("DefaultHandler")));
void BusFault_Handler(void)   __attribute__((weak, alias("DefaultHandler")));
void UsageFault_Handler(void) __attribute__((weak, alias("DefaultHandler")));
void SVC_Handler(void)        __attribute__((weak, alias("DefaultHandler")));
void DebugMon_Handler(void)   __attribute__((weak, alias("DefaultHandler")));
void PendSV_Handler(void)     __attribute__((weak, alias("DefaultHandler")));
void SysTick_Handler(void)    __attribute__((weak, alias("DefaultHandler")));

__attribute__((section(".vectors"), used))
void (*const _vectors[])(void) = {
    (void (*)(void))((uint32_t)&_estack), // initial stack pointer
    ResetHandler,                         // reset
    NMI_Handler,
    HardFault_Handler,
    MemManage_Handler,
    BusFault_Handler,
    UsageFault_Handler,
    0, 0, 0, 0,
    SVC_Handler,
    DebugMon_Handler,
    0,
    PendSV_Handler,
    SysTick_Handler,
    // (External IRQ handlers appended during peripheral bring-up.)
};

// ---------------------------------------------------------------------------
// 3. Reset entry point
// ---------------------------------------------------------------------------
__attribute__((section(".text"), naked, used))
void ResetHandler(void)
{
    // Set the stack pointer explicitly (the ROM may enter before SP is loaded).
    __asm__ volatile("ldr sp, =_estack");

    // Enable the FPU (CP10/CP11 full access) before any floating-point use.
    SCB->CPACR |= (3u << 20) | (3u << 22);
    __DSB();
    __ISB();

    // Copy .text from flash (XIP) into ITCM.
    uint32_t *src = &_flashstart_text;
    uint32_t *dst = &_stext;
    while (dst < &_etext) *dst++ = *src++;

    // Copy initialised data from flash into DTCM.
    src = &_flashstart_data;
    dst = &_sdata;
    while (dst < &_edata) *dst++ = *src++;

    // Zero the .bss section.
    dst = &_sbss;
    while (dst < &_ebss) *dst++ = 0;

    // Basic device init (watchdog disable, default clock gating) from the
    // NXP CMSIS system file.
    SystemInit();

    // Enable the L1 caches.
    SCB_EnableICache();
    SCB_EnableDCache();

    // TODO(hardware): configure the FlexRAM ITCM/DTCM bank split (IOMUXC_GPR
    // GPR14/16/17) to match the linker script, and bring up the full PLL /
    // clock tree (fsl_clock + a board clock_config). Validated on a real
    // Teensy 4.0 during bring-up; see docs/migration.

    __libc_init_array();   // C++ static constructors
    (void)main();
    while (1) { }
}
