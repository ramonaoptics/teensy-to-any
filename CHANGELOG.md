### Unreleased

* Add the ability to read from `uint8_t` hardware registers.

### 0.0.18

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
