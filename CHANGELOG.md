### 0.0.23 (2023/09/26)

* `gpio_pin_mode` now takes an optional parameter to set the value after the
  mode is set. This is useful for those setting output pins to avoid any delay
  in the serial communication that could set the pins in an invalid state
  for a lengthy period of time.

* Add new command `gpio_digital_pulse` and `gpio_digital_pulse_us` that enable
  the use to pulse a digital pin.

* Add new command `analog_pulse` that enable the use to pulse a an analog pin
  at a given duty cycle.

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
