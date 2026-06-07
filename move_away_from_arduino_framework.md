# Specification: Drop Teensy 3.2, move Teensy 4 off the Arduino framework

> **Status:** Decision document + provisional phased plan.
> This supersedes the earlier "strip LGPL files in place while staying on
> `framework = arduino`" plan that shipped as **0.22.0** (see `CHANGELOG.md` and
> `tools/license_strip.py`). That interim work is now obsolete: **Teensy 3.2 is
> being dropped**, and **Teensy 4.x moves to a non-GPL stack**.
>
> **The 0.x series is the last to support Teensy 3.2.** The first release on the
> new stack is **1.0.0**, Teensy-4-only and free of Arduino LGPL/GPL code.

---

## Part A — Decision document

### A.1 Problem

`framework-arduinoteensy` (Teensyduino) is a mix of licenses. PJRC's own core is
*mostly* MIT-like, but PJRC themselves state that "the Arduino software system
includes many libraries and files from many different contributors, and some of
the files are LGPL and GPL." In practice:

- Arduino-derived files (`Stream`, `WString`, `WMath`, `Time`, `IPAddress`,
  `Printable`, `WCharacter`, `Client`, `Server`, `wiring_private`, `EEPROM`)
  carry **LGPL 2.1+** headers.
- `libraries/SPI/SPI.{h,cpp}` is **dual GPL2 / LGPL2.1** — and is actively used
  by our protocol's `spi_*` command set.
- PJRC's core license also carries a "use solely with PJRC's Teensy products"
  clause for commercial redistribution.

The 0.22.0 interim approach (strip LGPL files at build time, replace with MIT
header-only shims, audit the rest) reduced the surface but left `SPI` on an
`ALLOWLIST` and still depends on the Arduino abstraction layer. The boundary is
fuzzy and fragile against upstream drift.

Two decisions resolve this cleanly:

1. **Drop Teensy 3.2 (MK20DX256).** It is discontinued by PJRC and is the
   worst-supported MCU across every non-GPL option. The 0.x series is its last
   supported firmware line.
2. **Build Teensy 4.x directly against a permissively-licensed stack** —
   `arm-none-eabi-gcc` + newlib + NXP's vendor SDK + TinyUSB — dropping the
   Arduino abstraction entirely.

With T3.2 gone, the product becomes Teensy-4-only and the entire shipped binary
can be free of Arduino LGPL/GPL code — making a real `1.0.0` milestone possible.

### A.2 Hard constraints

- **Protocol byte-identical.** The serial command set, syntax, semantics, error
  codes, and framing in `src/commandconstants.hpp` / `src/commandrouting.cpp`
  are frozen. No host-side change may be required (for Teensy 4 devices).
- **USB enumeration identical.** VID/PID, manufacturer/product descriptor
  strings, and the chip-ID-derived serial number must match today's T4 firmware
  so existing host drivers bind unchanged and the `serialnumber` command returns
  the same value.
- **Per-commit build-green** across all remaining (T4) CI environments — bisect
  must stay useful.

### A.3 Options researched (and the licenses)

| Option | License | Teensy 4.0 (iMXRT1062) | Verdict |
|---|---|---|---|
| **NXP MCUXpresso SDK** | BSD-3-Clause | First-class (RT1060 flagship) | **Chosen base** |
| **TinyUSB** | MIT | Solid iMXRT port | **Chosen for USB CDC** |
| **Zephyr RTOS** | Apache-2.0 | `boards/pjrc/teensy4` exists but *not actively maintained* | Rejected — heavy, unmaintained port |
| **Bare-metal CMSIS only** | Apache-2.0 + NXP BSD-3 headers | Hand-roll everything | Rejected — reinvents the NXP SDK by hand |
| **libopencm3 / ChibiOS** | LGPL / GPL | — | Rejected — these *are* the licenses we're fleeing |
| **Keep `framework = arduino` + strip (0.22.0)** | MIT shims + LGPL `SPI` on allowlist | works | Rejected — fuzzy boundary, retired with T3.2 |

Key research findings:
- There is **no actively-maintained, non-GPL framework with first-class support
  for both Teensy 3.2 and Teensy 4.0.** The K20/MK20DX256 is the weak link
  everywhere and the hardware is discontinued — which is what makes dropping it
  the right call.
- MCUXpresso SDK *is* the curated CMSIS + drivers + validated boot/clock for the
  iMXRT1062, under BSD-3. Hand-rolling bare metal would rebuild a worse version.
- TinyUSB (MIT) provides a clean-licensed CDC device stack with a proven iMXRT
  port, decoupling us from PJRC's USB code.

### A.4 Recommendation (decisions taken in this interview)

| Area | Decision |
|---|---|
| **Teensy 3.2** | **Dropped.** 0.x is its last firmware line. Envs `teensy32`, `teensy32_fan_hub` removed. |
| **T4 base** | NXP MCUXpresso SDK (BSD-3) for HAL/drivers/boot/clock |
| **USB CDC** | TinyUSB (MIT) |
| **Build system** | Stay on PlatformIO (`platform = teensy`); T4 envs drop the `framework =` line and compile the vendored SDK + TinyUSB + our `src/` via a custom build script |
| **I2C** | NXP `fsl_lpi2c` driver (BSD-3); re-map `i2c_*` commands onto it. `i2c_t3` and `teensy4_i2c` both removed. |
| **GPIO/SPI/ADC/PWM** | NXP `fsl_gpio` / `fsl_lpspi` / `fsl_adc` (LPADC) / `fsl_pwm` drivers |
| **SDK vendoring** | Fetch a **pinned** MCUXpresso SDK + TinyUSB at build time (PIO `lib_deps` / `extra_script`); not committed wholesale |
| **`src/` bridge** | Thin **MIT Arduino-compat shim** (`lib/arduino_compat/`) so `src/` compiles almost unchanged |
| **FastLED** | Make FastLED's iMXRT backend work via the compat shim; preserve the `fastled_*` commands |
| **Boot/flash-config/clock** | NXP SDK startup / `system_` / `clock_config` / FlexSPI NOR boot header (BSD-3), adapted to Teensy 4.0's flash layout so `teensy_loader` accepts the `.hex` |
| **`tools/license_strip.py`** | **Retired** — only existed for the framework build; no longer needed once T3.2/Arduino are gone |
| **Verification** | Build-green CI for all T4 envs + a documented **manual hardware sanity checklist** at merge; no new automated host test infra |
| **Version** | First release on the new stack is **1.0.0** — Teensy-4-only, free of Arduino LGPL/GPL code |

### A.5 What this achieves

- ✅ Teensy 4.x firmware built from BSD-3 / MIT / Apache-2.0 / newlib sources
  only — no Arduino LGPL/GPL abstraction layer, no PJRC core-license clause.
- ✅ A genuinely LGPL/GPL-free product line (the whole product is now T4).
- ✅ Same protocol, same USB identity, same host drivers on Teensy 4.
- ⚠️ Teensy 3.2 is **end-of-life** for this firmware. Deployed T3.2 units stay on
  the last 0.x release; no new firmware is published for them.

---

## Part B — Provisional phased implementation plan

**Branch:** `teensy4-no-arduino` (recommended — the goal has changed materially
from `remove-lgpl-framework-files`).

**Per-commit build-green policy:** every commit builds all remaining T4 envs. A
T4 env stays on the old Arduino build until the commit that flips it; T3.2 envs
are removed in Commit 1.

### B.0 Target file layout

```
lib/
  arduino_compat/          ← MIT, ours. Arduino dialect → NXP SDK / TinyUSB / SysTick
    library.json
    Arduino.h              ← umbrella: pins, timing, types
    src/
      wiring_digital.cpp   ← pinMode / digitalWrite / digitalRead → fsl_gpio
      wiring_analog.cpp    ← analogRead → fsl_adc (LPADC); analogWrite → fsl_pwm
      wiring_time.cpp      ← millis / micros / delay / delayMicroseconds → SysTick
      usb_serial.{h,cpp}   ← `Serial` (Print/Stream-compatible) → TinyUSB CDC
      Print.{h,cpp}        ← MIT clean-room (number formatting the protocol uses)
      Stream.{h,cpp}       ← MIT clean-room (read/available/peek/flush)
  mcux/                    ← vendored NXP SDK subset (fetched, pinned)
  tinyusb/                 ← vendored TinyUSB (fetched, pinned)
boards/
  teensy40_bare/           ← startup, linker script, FlexSPI boot header, clock_config
src/                       ← unchanged Arduino-dialect command code (recompiled)
tools/
  fetch_mcux.py            ← extra_script: pin + fetch SDK/TinyUSB
                           ← license_strip.py REMOVED
```

Removed from the repo / build: `teensy32` + `teensy32_fan_hub` envs, the
`i2c_t3` and `teensy4_i2c` dependencies, and `tools/license_strip.py`.

### B.1 Commit sequence

**Commit 1 — Drop Teensy 3.2 + toolchain/boot bring-up for one bare T4 env.**
- Remove `teensy32` and `teensy32_fan_hub` envs, the `i2c_t3` dependency, and
  `tools/license_strip.py` (+ its `extra_scripts` wiring).
- Add env `teensy40_bare` (parallel to `teensy40`, additive). Drop `framework =
  arduino`; add `tools/fetch_mcux.py` to fetch a pinned MCUXpresso SDK + TinyUSB.
- Vendor/adapt the NXP boot layer (`startup`, `system_MIMXRT1062`,
  `clock_config`, FlexSPI NOR boot header) and a linker script matching Teensy
  4.0's flash/RAM layout (ITCM/DTCM/OCRAM).
- Goal: a minimal `main()` that boots, blinks the LED, and is accepted/flashed
  by `teensy_loader`. **Brick risk is highest here** — verify on real hardware
  before proceeding.
- All remaining (T4) envs build green.

**Commit 2 — TinyUSB CDC with identical USB identity.**
- Bring up TinyUSB device + CDC class on `teensy40_bare`.
- Replicate the USB descriptors **exactly**: VID/PID (PJRC Teensy serial
  identity), manufacturer/product strings (`TEENSY_TO_ANY_MANUFACTURER_NAME`),
  and the iMXRT chip-ID-derived **serial number** so enumeration and the
  `serialnumber` command match today's output. This replaces the
  `usb_names.h` / `usb_string_descriptor_struct` weak-alias mechanism in
  `src/main.cpp`.
- Verify: host enumerates with byte-identical descriptors vs current firmware; a
  loopback echo over CDC works.

**Commit 3 — Arduino-compat shim (`lib/arduino_compat/`).**
- Implement the MIT shim covering the exact API surface `src/` uses:
  - `Serial` (Print/Stream) over TinyUSB CDC — every `Serial.print/println/
    write/read/available/peek/flush` call site, with number formatting written
    from scratch to match Arduino's output for the formats the protocol emits.
  - `pinMode` / `digitalWrite` / `digitalRead` → `fsl_gpio` with a Teensy-pin →
    (GPIO port, pin, IOMUX) mapping table for Teensy 4.0.
  - `millis` / `micros` / `delay` / `delayMicroseconds` → SysTick.
  - `analogRead` → `fsl_adc` (LPADC); `analogWrite` → `fsl_pwm` (FlexPWM/QTMR).
- Driver inventory step: enumerate each peripheral the command set touches and
  size only what's needed (no full HAL coverage).
- Build `src/` against the shim on `teensy40_bare`. No `src/` edits beyond what
  the shim can't cover.

**Commit 4 — I2C on `fsl_lpi2c`.**
- Re-map `src/i2c.cpp`'s `i2c_*` command implementations onto `fsl_lpi2c`
  (LPI2C1/3/4), removing the `teensy4_i2c` dependency.
- Match current semantics precisely: clock speed, timeouts, repeated-start,
  bulk transfer, and error/return codes. Highest protocol-parity risk after USB.

**Commit 5 — Flip the production T4 env(s) to the new stack.**
- Make `teensy40`, `teensy40_blinker`, `teensy40_fan_hub`,
  `teensy40_fan_hub_default_off` use the new stack (extend `teensy40_bare`,
  remove `framework = arduino`).
- `teensy40_fastled` is handled in Commit 6.
- All these envs build green and produce flashable `.hex`.

**Commit 6 — FastLED on the compat shim.**
- Extend `lib/arduino_compat/` enough that FastLED's iMXRT backend
  compiles/links (pin macros, timing, and any DMA/FlexIO hooks it expects).
  Preserve every `fastled_*` command.
- Fallback (open question, D): if the coupling is too deep, write a minimal MIT
  WS2812 driver covering only the exposed `fastled_*` commands.
- Flip `teensy40_fastled`; build green.

**Commit 7 — CI, docs, version 1.0.0.**
- Update `.github/workflows/build.yaml`: remove T3.2 jobs; T4 jobs build on the
  new stack. Keep publishing per-env `.hex`.
- README "Licensing" section: explain the LGPL/GPL-free posture, that the build
  is NXP SDK (BSD-3) + TinyUSB (MIT) + newlib, and that **Teensy 3.2 support
  ended with the 0.x series**.
- CHANGELOG `1.0.0` entry: "First LGPL/GPL-free release. Teensy 4.x is built
  directly against the NXP MCUXpresso SDK (BSD-3) and TinyUSB (MIT); the Arduino
  framework is gone. **Teensy 3.2 is no longer supported — the 0.x series is its
  last firmware line.** No protocol changes for Teensy 4 devices."
- Tag `v1.0.0` after merge; existing CI tag handler publishes the release.

### B.2 CI

- Remove T3.2 jobs. T4 jobs fetch the pinned SDK/TinyUSB → compile vendored
  stack + `src/` + compat shim. Same matrix shape otherwise.
- Pin discipline: `tools/fetch_mcux.py` pins exact SDK + TinyUSB commits; CI
  caches them. A pin bump is an explicit, reviewable commit.

### B.3 Verification

- **CI:** all remaining T4 envs build green every commit.
- **Manual hardware checklist at merge** (one Teensy 4.0), comparing the new
  firmware against the last Arduino-framework T4 build:
  1. Device enumerates with identical VID/PID + descriptor strings.
  2. `version`, `info`, `serialnumber` return expected values.
  3. `gpio_digital_write 13 1` / read-back behaves identically.
  4. An `i2c_*` round-trip against a known device matches.
  5. A `spi_*` round-trip matches.
  6. (fastled env) a `fastled_*` command drives the LEDs as before.
- No new automated host-side test infra is introduced (consistent with policy).

---

## Part C — Risks & mitigations

| Risk | Mitigation |
|---|---|
| iMXRT boot/FlexSPI/clock hand-off bricks boards. | Use NXP SDK validated boot/clock files (BSD-3); bring up + verify on hardware in Commit 1 before further work. |
| TinyUSB descriptors differ → host drivers don't bind / `serialnumber` changes. | Commit 2 replicates VID/PID + strings + chip-ID serial exactly; verify descriptors byte-for-byte. |
| `Serial.print` number formatting drifts from Arduino's. | Clean-room `Print` matches only the formats the protocol emits; covered by the manual output diff at merge. |
| `fsl_lpi2c` timing/error semantics differ from `teensy4_i2c`. | Map command-by-command; hardware round-trip check in B.3. |
| FastLED's iMXRT backend too coupled to the Teensy core to shim. | Fallback: minimal MIT WS2812 driver for the exposed `fastled_*` commands. |
| SDK/TinyUSB fetch fails or drifts. | Exact pinned commits + CI cache; pin bumps are explicit commits. |
| A field Teensy 3.2 needs a firmware change after EOL. | Documented: 0.x is the last T3.2 line. Any T3.2 change happens on a maintenance branch off the last 0.x tag, not on `main`. |

## Part D — Open questions

- **FastLED feasibility:** can the iMXRT FastLED backend be satisfied by a thin
  compat shim, or does it force the minimal-WS2812-driver fallback? Resolve in
  Commit 6; affects effort materially.
- **SPI driver scope:** confirm which `spi_*` features the protocol exercises
  (modes, clock dividers, `transfer16`, bulk) before sizing the `fsl_lpspi`
  mapping.
- **ADC/PWM parity:** confirm `analogRead`/`analogWrite` resolution, reference,
  and PWM frequency defaults match Arduino's so analog command output is
  unchanged.
- **`teensy_loader` `.hex` acceptance:** confirm the adapted FlexSPI boot header
  + load addresses produce a `.hex` the existing `teensy-cli` upload protocol
  flashes without changes.
- **Last-0.x maintenance branch:** decide whether to cut a `0.x` maintenance
  branch at the final T3.2 release for any emergency field fixes.
