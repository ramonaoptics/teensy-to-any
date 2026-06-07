# `lib/arduino_compat/` — required API surface

Every Arduino/framework symbol referenced by `src/`, with the non-GPL
replacement it must be backed by. Counts are call-site occurrences in `src/`
at the time of writing (`grep`-derived; re-run before implementing). Only what
`src/` actually uses needs implementing — nothing else.

## `Serial` — Print/Stream over TinyUSB CDC

| Symbol | Uses | Replacement |
|---|---|---|
| `Serial.print` | 18 | `Print::print` overloads → `tud_cdc_write` |
| `Serial.printf` | 3 | `Print::printf` (Teensy extension) → `vsnprintf` into the CDC write buffer |
| `Serial.read` | 2 | `Stream::read` → `tud_cdc_read_char` |
| `Serial.available` | 2 | `Stream::available` → `tud_cdc_available` |
| `Serial.send_now` | 1 | flush → `tud_cdc_write_flush` |
| `Serial.begin` | 1 | no-op (baud is irrelevant for USB CDC), keep signature |

Number formatting (`print(int/long/unsigned, base)`, `print(double, digits)`)
must be **written from scratch** and match Arduino's output for the formats the
protocol emits — this is the main protocol-parity risk in the shim.

## Digital / timing / analog

| Symbol | Uses | Replacement |
|---|---|---|
| `digitalWrite` | 9 | `fsl_gpio` `GPIO_PinWrite` (via pin map) |
| `digitalRead` | 1 | `GPIO_PinRead` |
| `pinMode` | 2 | IOMUX + `GPIO_PinInit` (direction/pull from mode) |
| `analogWrite` | 7 | `fsl_pwm` (FlexPWM / QTMR) |
| `analogWriteResolution` | 1 | PWM duty resolution config |
| `analogWriteFrequency` | 1 | PWM frequency config |
| `analogRead` | 1 | `fsl_adc` (LPADC) |
| `delay` | 4 | SysTick busy/sleep wait (ms) |
| `delayMicroseconds` | 3 | SysTick / cycle-counter wait (µs) |

Defaults that must match Arduino for byte-identical output: `analogRead`
resolution/reference and `analogWrite` frequency/resolution defaults.

## Constants / macros

`HIGH` (14), `LOW` (15), `OUTPUT` (9), `INPUT` (4), `MSBFIRST` (3),
`LSBFIRST` (2), `PROGMEM` (1). Provide as plain `#define`/`constexpr` in
`Arduino.h`. `PROGMEM` → empty (no AVR-style flash addressing on Cortex-M7).

## SPI — `fsl_lpspi`

`SPI.begin` / `SPI.end` / `SPI.beginTransaction` / `SPI.endTransaction` /
`SPI.transfer` (5) / `SPI.setSCK` / `SPI.setMOSI` / `SPI.setMISO`, plus
`SPISettings` and `SPI_MODE0..3`. Map to an `fsl_lpspi` master with a
`SPISettings`-compatible class (clock, bit order, mode). This also discharges
the original LGPL/GPL concern — Arduino's `SPI.cpp` is dual GPL2/LGPL2.1.

## EEPROM — flash-backed emulation

`EEPROM.read` (4) / `EEPROM.write` (5) / `EEPROM.update` (2) /
`EEPROM.length` (2). i.MX RT1062 has no hardware EEPROM. Provide a flash-backed
emulation of at least 1080 bytes (the demo-enabled flag is at address 1079).

## `src/main.cpp` USB descriptor hooks — the one expected `src/` change

`main.cpp` includes `<usb_names.h>` and defines `usb_string_manufacturer_name`
+ reads `usb_string_serial_number` (the `serialnumber` command). Under TinyUSB
this weak-alias mechanism is replaced by descriptor callbacks — see
[`usb-identity.md`](usb-identity.md). This is the only anticipated edit to
`src/` (everything else is satisfied by the shim).

## FastLED (`teensy40_fastled` only)

`#include "FastLED.h"` + `CRGB` / `CLEDController`. FastLED's i.MX RT backend is
tightly coupled to the Teensy core (pin macros, timing, DMA/FlexIO). Plan:
extend the compat shim until it links; fallback is a minimal MIT WS2812 driver
covering only the exposed `fastled_*` commands.
