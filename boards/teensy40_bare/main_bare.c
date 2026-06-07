// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Ramona Optics, Inc.
//
// Minimal placeholder application for the bare-metal Teensy 4.0 bring-up.
// This exists only to prove the no-framework build compiles and links to a
// valid .hex. Replaced by the real firmware (src/ + lib/arduino_compat) as the
// migration proceeds.

#include <stdint.h>
#include "tusb.h"

int main(void)
{
    // TODO(hardware): USB PHY/PLL3 clock bring-up and NVIC wiring of the
    // USB_OTG1 IRQ to tud_int_handler(0) happen during on-hardware bring-up.
    tusb_init();

    for (;;) {
        tud_task();   // TinyUSB device task
    }
    return 0;
}
