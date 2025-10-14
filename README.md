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

# Release Process

When developing and hacking on teensytoany, it is entirely possible to fully
use a local-first approach to programming and testing. However, when it comes
time to share modifications, it is important to follow a few steps to ensure
that the code is properly released and documented.

We keep track of "versions" using "tags" on github. The rest of the release
process typically happens through our CI/CD pipelines.

Often, a new release of the firmware is associated with a new release of the
python driver.

### New Teensy-To-Any firmware Release

1. Create a pull request with a new feature / documentation.
2. Ensure that the CIs pass. They compile a few of our examples and target different pieces of hardware.
3. Add a small note in the CHANGELOG.md file.
4. Merge the pull request.
5. Create a new tag on Github. The tag should be named X.Y.Z
    * where X is the major version
    * Y is the minor version
    * Z is the patch version

6. Allow github to generate the release notes. It does so by pulling in
   titles from the pull requests. This is mostly all that matters.

### New Python-TeensyToAny Release

Repeat the same process but on the python repo

https://github.com/ramonaoptics/python-teensytoany

IT should be quickly available on PyPI

https://pypi.org/project/teensytoany/

Once the tag is completed, follow through on conda-forge to make the release there

https://github.com/conda-forge/teensytoany-feedstock
