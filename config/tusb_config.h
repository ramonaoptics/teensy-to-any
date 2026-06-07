// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Ramona Optics, Inc.
//
// TinyUSB configuration for the bare-metal Teensy 4.0 (i.MX RT1062) CDC device.
// TinyUSB is configured entirely through this header; it must be on the include
// path (-I$PROJECT_DIR/config in platformio.ini). Only the CDC class is
// enabled; every other class/port self-disables via its CFG_* guard, so the
// unused TinyUSB sources compile to nothing.

#ifndef TUSB_CONFIG_H_
#define TUSB_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

// --- MCU / RTOS / speed ----------------------------------------------------
#define CFG_TUSB_MCU            OPT_MCU_MIMXRT1XXX
#define CFG_TUSB_OS             OPT_OS_NONE
#define CFG_TUSB_RHPORT0_MODE   (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED)
#define BOARD_TUD_RHPORT        0
#define BOARD_TUD_MAX_SPEED     OPT_MODE_HIGH_SPEED

#ifndef CFG_TUSB_DEBUG
#define CFG_TUSB_DEBUG          0
#endif

// DMA-capable, cache-line considerations handled by the chipidea ci_hs port.
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN      __attribute__((aligned(4)))

// --- Device stack ----------------------------------------------------------
#define CFG_TUD_ENABLED         1
#define CFG_TUD_ENDPOINT0_SIZE  64

// Only CDC (ACM serial) is used by this firmware.
#define CFG_TUD_CDC             1
#define CFG_TUD_MSC             0
#define CFG_TUD_HID             0
#define CFG_TUD_MIDI            0
#define CFG_TUD_VENDOR          0

#define CFG_TUD_CDC_RX_BUFSIZE  256
#define CFG_TUD_CDC_TX_BUFSIZE  256
#define CFG_TUD_CDC_EP_BUFSIZE  512

#ifdef __cplusplus
}
#endif

#endif // TUSB_CONFIG_H_
