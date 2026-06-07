// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Ramona Optics, Inc.
//
// Minimal placeholder application for the bare-metal Teensy 4.0 bring-up.
// This exists only to prove the no-framework build compiles and links to a
// valid .hex. Replaced by the real firmware (src/ + lib/arduino_compat) as the
// migration proceeds.

#include <stdint.h>

int main(void)
{
    volatile uint32_t counter = 0;
    for (;;) {
        counter++;
    }
    return 0;
}
