# Teensy-to-any

[![Build Status](https://travis-ci.org/ramonaoptics/teensy-to-any.svg?branch=master)](https://travis-ci.org/ramonaoptics/teensy-to-any)

This program allows you to use the Teensy as a USB to protocol converter.

The design is centered around exposing simple building blocks to a USB UART interface.
In this case, the Teensy is not designed as a standalone device. Rather, it should always be used with
a host computer that sends requests to accomplish different actions.

Currently, we support:


| Protocol | Teensy 3.2 | Teensy 4.0 |
|:---------|:-----------|:-----------|
| GPIO     | Yes        | Yes        |
| I2C      | Yes        | Yes        |
| PWM      | Yes        | Yes        |
| SPI      | Yes        | Yes        |

Future protocols include:

5. CAN
6. Serial

Generally speaking, this should help you prototype hardware, by bypassing the tedious steps of creating a USB Serial Parser, and letting you bring up hardware more naturally.

## Teensy 3.2

upload with

```
platformio run -e teensy32 --target upload
```

To upload a precompiled build:
```
teensy_loader_cli -s --mcu=TEENSY31 build_version_number.hex
```

## Teensy 4.0

upload with

```
platformio run -e teensy40 --target upload
```

To upload a precompiled build:
```
teensy_loader_cli -s --mcu=TEENSY40 build_version_number.hex
```
