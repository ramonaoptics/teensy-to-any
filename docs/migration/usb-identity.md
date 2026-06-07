# USB identity to reproduce under TinyUSB

The new TinyUSB CDC device **must enumerate identically** to today's firmware so
existing host drivers bind unchanged and the `serialnumber` command returns the
same value. All facts below are from the current `framework-arduinoteensy`
single-`Serial` (CDC) build plus the teensy-to-any override in `src/main.cpp`.

## Device descriptor

| Field | Value | Source |
|---|---|---|
| `idVendor` | `0x16C0` | `cores/teensy4/usb_desc.h` (`VENDOR_ID`, USB_SERIAL build) |
| `idProduct` | `0x0483` | `cores/teensy4/usb_desc.h` (`PRODUCT_ID`, USB_SERIAL build) |
| Device class | CDC (communications), single ACM interface | usb_desc CDC config |

> Only the single-`Serial` build (`0x16C0:0x0483`) matters. The other PID rows
> in `usb_desc.h` (Dual/Triple Serial, Keyboard, etc.) are unused — do not
> reproduce them.

## String descriptors

| Index | String | Notes |
|---|---|---|
| Manufacturer | `TeensyToAny` | Overridden in `src/main.cpp` via `TEENSY_TO_ANY_MANUFACTURER_NAME` (len 11), **not** the framework default `Teensyduino`. |
| Product | `USB Serial` | Framework default (`PRODUCT_NAME`, len 10); not overridden. |
| Serial number | decimal chip id | See derivation below. |

## Serial-number derivation (must match byte-for-byte)

From `cores/teensy4/usb_desc.c::usb_init_serialnumber()`:

```c
num = HW_OCOTP_MAC0 & 0xFFFFFF;        // low 24 bits of the OCOTP MAC0 fuse
if (num < 10000000) num = num * 10;    // OS-X CDC-ACM driver workaround
ultoa(num, buf, 10);                   // decimal ASCII, no leading zeros
// each ASCII char becomes a UTF-16 code unit in the string descriptor
```

Replicate exactly in the TinyUSB serial-string callback:
- read the same fuse (`HW_OCOTP_MAC0`, i.e. OCOTP bank `0x0400`, word `MAC0`),
- mask `& 0xFFFFFF`, apply the `< 10000000 ⇒ ×10` rule,
- format as base-10 ASCII, widen to UTF-16LE for the descriptor.

The `serialnumber` command in `src/main.cpp` reads `usb_string_serial_number`;
keep exposing the same value through whatever symbol/accessor the shim provides
so the command output is unchanged.

## Verification (Commit 2 of the plan)

Before/after on the same board, assert identical:
- `lsusb -v` (or `system_profiler SPUSBDataType` on macOS) device + string
  descriptors,
- the `serialnumber` command response,
- host driver binds and a CDC echo round-trips.
