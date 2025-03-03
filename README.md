# Teensy-to-any

[![Build Status](https://travis-ci.org/ramonaoptics/teensy-to-any.svg?branch=master)](https://travis-ci.org/ramonaoptics/teensy-to-any)

This program allows you to use the Teensy as a USB to protocol converter.

The design is centered around exposing simple building blocks to a USB UART interface.
In this case, the Teensy is not designed as a standalone device. Rather, it should always be used with
a host computer that sends requests to accomplish different actions.

Currently, we support:


| Protocol | Teensy 4.0 |
|:---------|:-----------|
| GPIO     | Yes        |
| I2C      | Yes        |
| PWM      | Yes        |
| SPI      | Yes        |

Future protocols include:

5. CAN
6. Serial

Generally speaking, this should help you prototype hardware, by bypassing the tedious steps of creating a USB Serial Parser, and letting you bring up hardware more naturally.

## Teensy 4.0

upload with

```
platformio run -e teensy40 --target upload
```

To upload a precompiled build:
```
teensy_loader_cli -s --mcu=TEENSY40 build_version_number.hex
```

This library also provides a basic "setup" and loop capabilities. This can be
useful if you want to create an open-loop demonstration application. No "if"
statements are used in the loop meaning it is very difficult to create
conditions.

* `teensy40_blinder`: blinks the built in LED of the Teensy40 a few times for your pleasure!
* `teensy40_fan_hub`: shows how the teensy can control 4 pin fans at different speeds.
* `teensy40_fan_hub_default_off`: setups up 4 pin fan control on Pin 2. After bootup, fans are off.
* `teensy40_neopixel_example`: Drives 5 neopixel RGBW LEDs.


<details><summary>Deprecation of Teensy3</summary>

## Teensy 3.2

| Protocol | Teensy 3.2 |
|:---------|:-----------|
| GPIO     | Yes        |
| I2C      | Yes        |
| PWM      | Yes        |
| SPI      | Yes        |

upload with

```
platformio run -e teensy32 --target upload
```

To upload a precompiled build:
```
teensy_loader_cli -s --mcu=TEENSY31 build_version_number.hex
```

</details>
