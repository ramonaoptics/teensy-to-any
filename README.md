# Teensy-to-any

[![Build Status](https://travis-ci.org/ramonaoptics/teensy-to-any.svg?branch=master)](https://travis-ci.org/ramonaoptics/teensy-to-any)

This program allows you to use the Teensy as a USB to protocol converter.

The design is centered around exposing simple building blocks to a USB UART interface.
In this case, the Teensy is not designed as a standalone device. Rather, it should always be used with 
a host computer that sends requests to accomplish different actions.

Currently, we support:

1. GPIO
2. I2C
3. PWM
4. SPI

Future protocols include:
5. CAN
6. Serial


Generally speaking, this should help you prototype hardware, by bypassing the tedious steps of creating a USB Serial Parser, and letting you bring up hardware more naturally.

