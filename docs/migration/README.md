# Teensy 4 → non-GPL stack: migration kickoff

These documents are the code-grounded inputs for moving Teensy 4.x off the
Arduino framework onto NXP MCUXpresso SDK (BSD-3) + TinyUSB (MIT), as decided in
[`../../move_away_from_arduino_framework.md`](../../move_away_from_arduino_framework.md).

They exist so the boot/USB/driver/shim work can be done (by humans or agents)
against precise, verified facts rather than guesses. Generated from the current
`src/` tree and the pinned `framework-arduinoteensy` package.

## Index

| Doc | Purpose |
|---|---|
| [`arduino-compat-surface.md`](arduino-compat-surface.md) | Every Arduino/framework symbol `src/` uses, mapped to its non-GPL replacement. This is the spec for `lib/arduino_compat/`. |
| [`usb-identity.md`](usb-identity.md) | The exact VID/PID, descriptor strings, and serial-number derivation TinyUSB must reproduce byte-for-byte. |

## Hard invariants (do not break)

- **Serial protocol byte-identical** — command set, syntax, return formats, and
  error codes in `src/commandconstants.hpp` / `src/commandrouting.cpp` are frozen.
- **USB identity identical** — see `usb-identity.md`. Existing host drivers must
  bind unchanged and the `serialnumber` command must return the same value.
- **Per-commit build-green** for all Teensy 4 envs (`teensy40`,
  `teensy40_blinker`, `teensy40_fan_hub`, `teensy40_fastled`,
  `teensy40_fan_hub_default_off`).

## Still to inventory

- **Teensy 4.0 pin → IOMUX/GPIO/ADC/PWM map.** The `gpio_*`, `pwm_*`, and
  `analog_*` commands can address arbitrary pins, so the full 0–39 pin map must
  be reproduced (source of truth: PJRC `cores/teensy4/pins_arduino.h` +
  `core_pins.h`, cross-checked against the Teensy 4.0 schematic / i.MX RT1062
  reference manual — reproduce as our own table, do not copy LGPL code).
- **EEPROM emulation.** i.MX RT1062 has no hardware EEPROM; PJRC emulates ~1080
  bytes in flash. `src/` uses `EEPROM.{read,write,update,length}` (the demo flag
  lives at address 1079), so the compat layer needs a flash-backed emulation
  (NXP has an EEPROM-emulation driver; or implement a small reserved sector).
