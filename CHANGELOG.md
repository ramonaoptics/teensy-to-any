### 0.20.0 (2025/08/30)

* Added back `fastled_set_hsv` and `fastled_set_hue` commands with fixes for FastLED library compatibility.

### 0.19.0 (2025/08/30)

* Remove command `fastled_set_hsv` and `fastled_set_hue` as due to breakage in
  FastLED library.

### 0.18.0 (2025/08/22)

* Provide new command ``spi_transfer16``.
* ``spi_transfer`` now returns the read value for the uint8.

### 0.17.0 (2025/08/22)

* Provide a method to control the fastled refresh rate.

### 0.16.0 (2025/08/21)

* Fix I2C overflow handling by standardizing maximum buffer size to 256 bytes across all I2C functions.
* Add missing overflow checks to prevent buffer overflows in i2c_write and i2c_1_write functions.
* Update command documentation to accurately reflect function signatures and parameter requirements.
* Fix incorrect bit size descriptions in documentation (8-bit vs 16-bit functions).
* Correct parameter syntax for transaction-based I2C commands.
* Reduce BUFFER_SIZE from 16KB to 2KB for improved memory efficiency and Teensy 3.2 compatibility.

### 0.15.0 (2025/08/11)

* Reduce number of LEDs from 50 to 6 in the fastled demo code.

### 0.14.0 (2025/06/16)

* Provide ways to break up an i2c transaction with very low level i2c commands.

### 0.13.0 (2025/06/14)

* The usb manfuacturer is now specified as TeensyToAny instead of Teensyduino.

### 0.12.0 (2025/05/26)

* Beta: Provide an i2c ping command. At the time of release this feature has
  not been tested. We value user feedback as well as any bug reports.

### 0.11.0 (2025/05/26)

* Allow payloads of up to 16 instead of 8 for i2c.

### 0.10.0 (2025/03/12)

* Add support for Wire1 interface on Teensy3.2 and Teensy4.0.

### 0.9.0 (2025/03/09)

* Fix edge case with FastLED driver.

### 0.8.0 (2025/03/09)

* Provide basic support for FastLED.
* Provide a way to run commands immediately after the Serial Port has been setup.

### 0.7.0 (2025/03/03)

* Provide the ability to use Neopixels.

### 0.6.0 (2024/12/11)

* Fix the usecase where an infinite loop would occur if no demo commands.

### 0.5.0 (2024/11/10)

* Provide the capability to define startup and demo commands.

### 0.4.0 (2024/01/04)

* Provide EEPROM commands.
* Provide a method for the Teensy to read it's own USB Serial Number back to
  the device through the USB interface.

### 0.3.0 (2023/11/06)

* Increase the maximum analog pulse duration to 3 minutes.

### 0.2.0 (2023/11/03)

* Increase the maximum digital pulse duration to 3 minutes.

### 0.1.0 (2023/09/29)

* hex files with precise versions are uploaded enabling better version pinning.

### 0.0.23 (2023/09/26)

* `gpio_pin_mode` now takes an optional parameter to set the value after the
  mode is set. This is useful for those setting output pins to avoid any delay
  in the serial communication that could set the pins in an invalid state
  for a lengthy period of time.

* Add new command `gpio_digital_pulse` and `gpio_digital_pulse_us` that enable
  the use to pulse a digital pin.

* Add new command `analog_pulse` that enable the use to pulse a an analog pin
  at a given duty cycle.

* `spi_transfer_bulk` now also returns the data read back from device.

### 0.0.22 (2023/09/24)

* Add the command `i2c_read_payload_no_register` to provide the ability to read
  from devices like the AT24CS01 EEPROM data over I2C.
* Add the command `i2c_read_payload_uint16`.

### 0.0.21

* Add support for I2C on the Teensy4 through the use of
  [Richard Gemmell's library](https://github.com/Richard-Gemmell/teensy4_i2c/).

### 0.0.20

* Add the ability to read from `uint8_t`, `uint16_t`, and `uint32_t` hardware
  registers.

### 0.0.19

* Use send_now to trigger a communication event once a command has been processed.

### 0.0.17

* Add the ability to do SPI byte reads

### 0.0.16

* Bugfix: Ensure that analog reads return unsigned values.

### 0.0.15

* Add the ability to do analog reads.

### 0.0.14

* Enable reading and writing a contiguous payload of bytes

### 0.0.13

* Ensure that the system is compiled with I2C support for Teensy 3.2

### 0.0.12

* Preliminary Teensy 4.0 Support

### 0.0.11

* Added the ability to talk to 1 byte register address  I2C devices.
### 0.0.10

* Fix spi_transfer_bulk
### 0.0.9

* Enable control of SPI through the interface.

### 0.0.8

* Use git describe --tag to correctly identify the tags

### 0.0.7

* Fixed up version string in the Teensy C++ Code.
* Fixed up parsing spi_settings, now requires 4 parameters.
