#pragma once

#include "commandrouting.hpp"

int info_func(CommandRouter *cmd, int argc, const char **argv);
#if defined(TEENSYDUINO)
int reboot_func(CommandRouter *cmd, int argc, const char **argv);
#endif
int version_func(CommandRouter *cmd, int argc, const char **argv);
int mcu_func(CommandRouter *cmd, int argc, const char **argv);
int serialnumber_func(CommandRouter *cmd, int argc, const char **argv);
int command_license_func(CommandRouter *cmd, int argc, const char **argv);

// Digital GPIO
int gpio_pin_mode(CommandRouter *cmd, int argc, const char **argv);
int gpio_digital_write(CommandRouter *cmd, int argc, const char **argv);
int gpio_digital_read(CommandRouter *cmd, int argc, const char **argv);
int gpio_digital_pulse(CommandRouter *cmd, int argc, const char **argv);

// I2C support
int i2c_init(CommandRouter *cmd, int argc, const char **argv);
int i2c_reset(CommandRouter *cmd, int argc, const char **argv);
int i2c_write_uint16(CommandRouter *cmd, int argc, const char **argv);
int i2c_read_uint16(CommandRouter *cmd, int argc, const char **argv);
int i2c_write_uint8(CommandRouter *cmd, int argc, const char **argv);
int i2c_read_uint8(CommandRouter *cmd, int argc, const char **argv);
int i2c_read_no_register_uint8(CommandRouter *cmd, int argc, const char **argv);
int i2c_write_no_register_uint8(CommandRouter *cmd, int argc,
                                const char **argv);
int i2c_write_payload(CommandRouter *cmd, int argc, const char **argv);
int i2c_read_payload(CommandRouter *cmd, int argc, const char **argv);
int i2c_read_payload_no_register(CommandRouter *cmd, int argc, const char **argv);
int i2c_read_payload_uint16(CommandRouter *cmd, int argc, const char **argv);

int i2c_begin_transaction(CommandRouter *cmd, int argc, const char **argv);
int i2c_write(CommandRouter *cmd, int argc, const char **argv);
int i2c_end_transaction(CommandRouter *cmd, int argc, const char **argv);

int i2c_ping(CommandRouter *cmd, int argc, const char **argv);
int i2c_buffer_size(CommandRouter *cmd, int argc, const char **argv);

#if defined(TEENSYDUINO)
int i2c_1_init(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_reset(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_write_uint16(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_read_uint16(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_write_uint8(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_read_uint8(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_read_no_register_uint8(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_write_no_register_uint8(CommandRouter *cmd, int argc,
                                const char **argv);
int i2c_1_write_payload(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_read_payload(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_read_payload_no_register(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_read_payload_uint16(CommandRouter *cmd, int argc, const char **argv);

int i2c_1_begin_transaction(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_write(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_end_transaction(CommandRouter *cmd, int argc, const char **argv);

int i2c_1_ping(CommandRouter *cmd, int argc, const char **argv);
int i2c_1_buffer_size(CommandRouter *cmd, int argc, const char **argv);
#endif

// PWM Support
int analog_write(CommandRouter *cmd, int argc, const char **argv);
#if defined(TEENSYDUINO)
int analog_write_frequency(CommandRouter *cmd, int argc, const char **argv);
int analog_write_resolution(CommandRouter *cmd, int argc, const char **argv);
#endif
int analog_pulse(CommandRouter *cmd, int argc, const char **argv);

// Analog IO
int analog_read(CommandRouter *cmd, int argc, const char **argv);

// SPI support
int spi_begin(CommandRouter *cmd, int argc, const char **argv);
int spi_end(CommandRouter *cmd, int argc, const char **argv);
#if defined(TEENSYDUINO)
int spi_set_mosi(CommandRouter *cmd, int argc, const char **argv);
int spi_set_miso(CommandRouter *cmd, int argc, const char **argv);
int spi_set_sck(CommandRouter *cmd, int argc, const char **argv);
#endif
int spi_settings(CommandRouter *cmd, int argc, const char **argv);
#if defined(TEENSYDUINO)
int spi_begin_transaction(CommandRouter *cmd, int argc, const char **argv);
int spi_end_transaction(CommandRouter *cmd, int argc, const char **argv);
#endif
int spi_transfer(CommandRouter *cmd, int argc, const char **argv);
int spi_transfer16(CommandRouter *cmd, int argc, const char **argv);
int spi_read_byte(CommandRouter *cmd, int argc, const char **argv);
int spi_transfer_bulk(CommandRouter *cmd, int argc, const char **argv);
int spi_buffer_size(CommandRouter *cmd, int argc, const char **argv);
int spi_set_clock_divider(CommandRouter *cmd, int argc, const char **argv);

int register_read_uint8(CommandRouter *cmd, int argc, const char **argv);
int register_write_uint8(CommandRouter *cmd, int argc, const char **argv);
int register_read_uint16(CommandRouter *cmd, int argc, const char **argv);
int register_write_uint16(CommandRouter *cmd, int argc, const char **argv);
int register_read_uint32(CommandRouter *cmd, int argc, const char **argv);
int register_write_uint32(CommandRouter *cmd, int argc, const char **argv);

int eeprom_read_uint8(CommandRouter *cmd, int argc, const char **argv);
int eeprom_write_uint8(CommandRouter *cmd, int argc, const char **argv);

#if defined(TEENSYDUINO)
int fastled_add_leds(CommandRouter *cmd, int argc, const char **argv);
int fastled_show(CommandRouter *cmd, int argc, const char **argv);
int fastled_set_rgb(CommandRouter *cmd, int argc, const char **argv);
int fastled_set_hsv(CommandRouter *cmd, int argc, const char **argv);
int fastled_set_hue(CommandRouter *cmd, int argc, const char **argv);
int fastled_set_brightness(CommandRouter *cmd, int argc, const char **argv);
int fastled_get_brightness(CommandRouter *cmd, int argc, const char **argv);
int fastled_set_max_refresh_rate(CommandRouter *cmd, int argc, const char **argv);
#endif

// Mostly for debugging and startup scripts
int sleep_seconds(CommandRouter *cmd, int argc, const char **argv);

int startup_commands_available(CommandRouter *cmd, int argc, const char **argv);
int read_startup_command(CommandRouter *cmd, int argc, const char **argv);
int post_serial_startup_commands_available(CommandRouter *cmd, int argc, const char **argv);
int read_post_serial_startup_command(CommandRouter *cmd, int argc, const char **argv);

int read_demo_command(CommandRouter *cmd, int argc, const char **argv);
int disable_demo_commands(CommandRouter *cmd, int argc, const char **argv);
int enable_demo_commands(CommandRouter *cmd, int argc, const char **argv);
int demo_commands_available(CommandRouter *cmd, int argc, const char **argv);
int demo_commands_enabled(CommandRouter *cmd, int argc, const char **argv);

int nop_func(CommandRouter *cmd, int argc, const char **argv);

#if !defined(TEENSYDUINO) && defined(ARDUINO) && defined(__AVR__)
#   define COMMAND_DEFINITION(name, description, syntax, func) {name, func}
#else
#   define COMMAND_DEFINITION(name, description, syntax, func) {name, description, syntax, func}
#endif

// Syntax is: {short command, description, syntax}
const command_item_t command_list[] = {
#if defined(TEENSYDUINO)
    COMMAND_DEFINITION("?", "Display help info", "?", command_help_func),
    COMMAND_DEFINITION("info", "Displays information about this TeensyToAny device", "about", info_func),
    COMMAND_DEFINITION("reboot", "Runs setup routine again, for this device.", "reboot", reboot_func),
    COMMAND_DEFINITION("serialnumber", "Displays the serial number of the board.", "serialnumber", serialnumber_func),
    COMMAND_DEFINITION("license",
      "Display the license information for the source code running on the "
      "teensy",     "license", command_license_func),
#endif
    COMMAND_DEFINITION("mcu", "Displays information about the microcontroller board.", "mcu", mcu_func),
    COMMAND_DEFINITION("version", "Display controller version number", "version", version_func),
    COMMAND_DEFINITION("gpio_pin_mode",
     "Set the pinMode of a GPIO pin, 0 for INPUT, 1 for OUTPUT. "
     "Optionally, the value can be provided. If the value is provided "
     "a digitalWrite will be issued.",
     "gpio_pin_mode pin mode [value]", gpio_pin_mode),
    COMMAND_DEFINITION("gpio_digital_write",
     "Set the output of a GPIO pin, 0 for LOW, 1 for HIGH",
     "gpio_digital_write pin value", gpio_digital_write),
    COMMAND_DEFINITION("gpio_digital_read", "Read the value of a GPIO pin, 0 for LOW, 1 for HIGH",
     "gpio_digital_read pin", gpio_digital_read),
    COMMAND_DEFINITION("gpio_digital_pulse",
     "Pulse the output of a GPIO pin for a given duration in seconds.",
     "gpio_digital_pulse pin value value_end duration_us", gpio_digital_pulse),
    COMMAND_DEFINITION("i2c_init", "Initialize I2C Communication",
     "i2c_init [baudrate=100_000] [timeout_ms=200_000] [address_size=2] "
     "[address_msb_first=1]",
     i2c_init),
    COMMAND_DEFINITION("i2c_reset", "Reset the I2C PORT in case of lockup.", "i2c_reset",
     i2c_reset),
    COMMAND_DEFINITION("i2c_write_uint16", "Write a 16 bit number to the I2C device",
     "i2c_write_uint16 slave_address register_address data", i2c_write_uint16),
    COMMAND_DEFINITION("i2c_read_uint16", "Read a 16 bit number from the I2C device",
     "i2c_read_uint16 slave_address register_address", i2c_read_uint16),
    COMMAND_DEFINITION("i2c_write_uint8", "Write an 8 bit number to the I2C device",
     "i2c_write_uint8 slave_address register_address data", i2c_write_uint8),
    COMMAND_DEFINITION("i2c_read_uint8", "Read an 8 bit number from the I2C device",
     "i2c_read_uint8 slave_address register_address", i2c_read_uint8),
    COMMAND_DEFINITION("i2c_read_no_register_uint8",
     "Read a uint8_t from the I2C bus without specifying a register address.",
     "i2c_read_no_register_uint8 slave_address", i2c_read_no_register_uint8),
    COMMAND_DEFINITION("i2c_write_no_register_uint8",
     "Write a uint8_t to the I2C bus without specifying a register address.",
     "i2c_write_no_register_uint8 slave_address data",
     i2c_write_no_register_uint8),
    COMMAND_DEFINITION("i2c_write_payload",
     "Write up to 256 bytes to the I2C bus starting at a specified register "
     "address.",
     "i2c_write_payload slave_address register_address data0 data1 ...",
     i2c_write_payload),
    COMMAND_DEFINITION("i2c_read_payload",
     "Read up to 256 bytes from the I2C bus starting at a specified 8 bit "
     "register address.",
     "i2c_read_payload slave_address register_address num_bytes",
     i2c_read_payload),
    COMMAND_DEFINITION("i2c_read_payload_no_register",
     "Read up to 256 bytes from the I2C bus without specifying a register "
     "address.",
     "i2c_read_payload_no_register slave_address num_bytes",
     i2c_read_payload_no_register),
    COMMAND_DEFINITION("i2c_read_payload_uint16",
      "Read up to 256 bytes from the I2C bus starting at a specified 16 bit "
      "register address.",
      "i2c_read_payload_uint16 slave_address register_address num_bytes",
      i2c_read_payload_uint16),
    COMMAND_DEFINITION("i2c_begin_transaction",
     "Begin a transaction with the I2C device, "
     "this is required before writing or reading data.",
     "i2c_begin_transaction slave_address", i2c_begin_transaction),
    COMMAND_DEFINITION("i2c_write", "Write data to the I2C device",
     "i2c_write data0 data1 ...", i2c_write),
    COMMAND_DEFINITION("i2c_end_transaction",
     "End a transaction with the I2C device, "
     "this is required after writing or reading data.",
     "i2c_end_transaction [stop=true]", i2c_end_transaction),
    COMMAND_DEFINITION("i2c_ping",
     "Ping the bus to check if the address acknoledges a read request.",
     "i2c_ping slave_address",
      i2c_ping),
    COMMAND_DEFINITION("i2c_buffer_size",
     "Get the maximum I2C buffer size for this board.",
     "i2c_buffer_size",
      i2c_buffer_size),
#if defined(TEENSYDUINO)
    COMMAND_DEFINITION("i2c_1_init", "Initialize I2C Communication",
    "i2c_1_init [baudrate=100_000] [timeout_ms=200_000] [address_size=2] "
    "[address_msb_first=1]",
    i2c_1_init),
    COMMAND_DEFINITION("i2c_1_reset", "Reset the I2C PORT in case of lockup.", "i2c_1_reset",
    i2c_1_reset),
    COMMAND_DEFINITION("i2c_1_write_uint16", "Write a 16 bit number to the I2C device",
    "i2c_1_write_uint16 slave_address register_address data", i2c_1_write_uint16),
    COMMAND_DEFINITION("i2c_1_read_uint16", "Read a 16 bit number from the I2C device",
    "i2c_1_read_uint16 slave_address register_address", i2c_1_read_uint16),
    COMMAND_DEFINITION("i2c_1_write_uint8", "Write an 8 bit number to the I2C device",
    "i2c_1_write_uint8 slave_address register_address data", i2c_1_write_uint8),
    COMMAND_DEFINITION("i2c_1_read_uint8", "Read an 8 bit number from the I2C device",
    "i2c_1_read_uint8 slave_address register_address", i2c_1_read_uint8),
    COMMAND_DEFINITION("i2c_1_read_no_register_uint8",
    "Read a uint8_t from the I2C bus without specifying a register address.",
    "i2c_1_read_no_register_uint8 slave_address", i2c_1_read_no_register_uint8),
    COMMAND_DEFINITION("i2c_1_write_no_register_uint8",
    "Write a uint8_t to the I2C bus without specifying a register address.",
    "i2c_1_write_no_register_uint8 slave_address data",
    i2c_1_write_no_register_uint8),
    COMMAND_DEFINITION("i2c_1_write_payload",
    "Write up to 256 bytes to the I2C bus starting at a specified register "
    "address.",
    "i2c_1_write_payload slave_address register_address data0 data1 ...",
    i2c_1_write_payload),
    COMMAND_DEFINITION("i2c_1_read_payload",
    "Read up to 256 bytes from the I2C bus starting at a specified 8 bit "
    "register address.",
    "i2c_1_read_payload slave_address register_address num_bytes",
    i2c_1_read_payload),
    COMMAND_DEFINITION("i2c_1_read_payload_no_register",
    "Read up to 256 bytes from the I2C bus without specifying a register "
    "address.",
    "i2c_1_read_payload_no_register slave_address num_bytes",
    i2c_1_read_payload_no_register),
    COMMAND_DEFINITION("i2c_1_read_payload_uint16",
        "Read up to 256 bytes from the I2C bus starting at a specified 16 bit "
        "register address.",
        "i2c_1_read_payload_uint16 slave_address register_address num_bytes",
        i2c_1_read_payload_uint16),
    COMMAND_DEFINITION("i2c_1_begin_transaction",
      "Begin a transaction with the I2C device, "
      "this is required before writing or reading data.",
      "i2c_1_begin_transaction slave_address", i2c_1_begin_transaction),
    COMMAND_DEFINITION("i2c_1_write", "Write data to the I2C device",
      "i2c_1_write data0 data1 ...", i2c_1_write),
    COMMAND_DEFINITION("i2c_1_end_transaction",
      "End a transaction with the I2C device, "
      "this is required after writing or reading data.",
      "i2c_1_end_transaction [stop=true]", i2c_1_end_transaction),
    COMMAND_DEFINITION("i2c_1_ping",
     "Ping the bus to check if the address acknoledges a read request.",
     "i2c_1_ping slave_address",
      i2c_1_ping),
    COMMAND_DEFINITION("i2c_1_buffer_size",
     "Get the maximum I2C buffer size for this board.",
     "i2c_1_buffer_size",
      i2c_1_buffer_size),
#endif
    COMMAND_DEFINITION("analog_write", "Write the duty cycle of the PWM",
     "analog_write pin dutycycle", analog_write),
#if defined(TEENSYDUINO)
    COMMAND_DEFINITION("analog_write_frequency", "Write the frequency of the PWM",
     "analog_write_frequency pin frequency", analog_write_frequency),
    COMMAND_DEFINITION("analog_write_resolution", "Write the resolution of the PWM timer",
     "analog_write_resolution bitdepth", analog_write_resolution),
#endif
    COMMAND_DEFINITION("analog_pulse", "Write the duty cycle of the PWM for a given duration of time.",
     "analog_pulse pin dutycycle dutycycle_end duration", analog_pulse),
    COMMAND_DEFINITION("analog_read", "Read the value of an analog pin, 0-255",
     "analog_read pin", analog_read),
    COMMAND_DEFINITION("spi_begin", "SPI Begin", "spi_begin", spi_begin),
    COMMAND_DEFINITION("spi_end", "SPI End", "spi_end", spi_begin),
#if defined(TEENSYDUINO)
    COMMAND_DEFINITION("spi_set_mosi", "SPI set MOSI", "spi_set_mosi mosi", spi_set_mosi),
    COMMAND_DEFINITION("spi_set_miso", "SPI set MISO ", " spi_set_miso miso", spi_set_miso),
    COMMAND_DEFINITION("spi_set_sck", "SPI set SCK ", " spi_set_sck sck", spi_set_sck),
#endif
    COMMAND_DEFINITION("spi_settings", "SPI set setting",
     "spi_settings frequency bitOrder dataMode", spi_settings),

#if defined(TEENSYDUINO)
    COMMAND_DEFINITION("spi_begin_transaction", "spi_begin_transaction", "spi_begin_transaction",
     spi_begin_transaction),
    COMMAND_DEFINITION("spi_end_transaction", "spi_end_transaction", "spi_end_transaction",
     spi_end_transaction),
#endif
    COMMAND_DEFINITION("spi_transfer", "SPI Transfer 8bits of data", "spi_transfer data",
     spi_transfer),
    COMMAND_DEFINITION("spi_transfer16", "SPI Transfer 16bits of data", "spi_transfer16 data",
     spi_transfer16),
    COMMAND_DEFINITION("spi_read_byte", "SPI transfer register address and read a byte",
     "spi_read_byte data", spi_read_byte),
    COMMAND_DEFINITION("spi_transfer_bulk", "SPI transfer multiple sets of 8 bits of data",
     "spi_transfer_bulk data[0] data[1] data[2] [...]", spi_transfer_bulk),
    COMMAND_DEFINITION("spi_buffer_size",
     "Get the maximum SPI buffer size for this board.",
     "spi_buffer_size",
      spi_buffer_size),
    COMMAND_DEFINITION("register_read_uint8", "Read an arbitrary hardware register.",
     "register_read_uint8 address", register_read_uint8),
    COMMAND_DEFINITION("register_write_uint8", "Write to an arbitrary hardware register.",
     "register_write_uint8 address data", register_write_uint8),
    COMMAND_DEFINITION("register_read_uint16", "Read an arbitrary hardware register.",
     "register_read_uint16 address", register_read_uint16),
    COMMAND_DEFINITION("register_write_uint16", "Write to an arbitrary hardware register.",
     "register_write_uint16 address data", register_write_uint16),
    COMMAND_DEFINITION("register_read_uint32", "Read an arbitrary hardware register.",
     "register_read_uint32 address", register_read_uint32),
    COMMAND_DEFINITION("register_write_uint32", "Write to an arbitrary hardware register.",
     "register_write_uint32 address data", register_write_uint32),
    COMMAND_DEFINITION("eeprom_read_uint8", "Read data from a given EEPROM address.",
     "eeprom_read_uint8 address", eeprom_read_uint8),
    COMMAND_DEFINITION("eeprom_write_uint8", "Write to an EEPROM address.",
     "eeprom_write_uint8 address data", eeprom_write_uint8),
#if defined(TEENSYDUINO)
    COMMAND_DEFINITION("fastled_add_leds", "Initialize the FastLED library",
     "fastled_init num_pixels pin type", fastled_add_leds),
    COMMAND_DEFINITION("fastled_show", "Show the current FastLED buffer",
     "fastled_show [scale]", fastled_show),
    COMMAND_DEFINITION("fastled_set_rgb", "Set the color of a pixel in the FastLED buffer",
     "fastled_set_rgb pixel red green blue", fastled_set_rgb),
    COMMAND_DEFINITION("fastled_set_hsv", "Set the color of a pixel in the FastLED buffer",
     "fastled_set_hsv pixel hue saturation value", fastled_set_hsv),
    COMMAND_DEFINITION("fastled_set_hue", "Set the hue of the FastLED buffer",
     "fastled_set_hue pixel hue", fastled_set_hue),
    COMMAND_DEFINITION("fastled_set_brightness", "Set the brightness of the FastLED buffer",
     "fastled_set_brightness scale", fastled_set_brightness),
    COMMAND_DEFINITION("fastled_get_brightness", "Get the brightness of the FastLED buffer",
     "fastled_get_brightness", fastled_get_brightness),
    COMMAND_DEFINITION("fastled_set_max_refresh_rate", "Set the maximum refresh rate of the FastLED buffer",
     "fastled_set_max_refresh_rate rate", fastled_set_max_refresh_rate),
#endif
    COMMAND_DEFINITION("sleep", "Sleep (and block) for the desired duration",
     "sleep duration", sleep_seconds),
    COMMAND_DEFINITION("startup_commands_available", "Number of startup commands available",
     "startup_commands_available", startup_commands_available),
    COMMAND_DEFINITION("read_startup_command", "Read a startup command",
     "read_startup_command index", read_startup_command),
    COMMAND_DEFINITION("post_serial_startup_commands_available", "Number of post serial startup commands available",
     "post_serial_startup_commands_available", startup_commands_available),
    COMMAND_DEFINITION("read_post_serial_startup_command", "Read a post serial startup command",
     "read_post_serial_startup_command index", read_startup_command),
    COMMAND_DEFINITION("demo_commands_available", "Number of demo commands available",
     "demo_commands_available", demo_commands_available),
    COMMAND_DEFINITION("read_demo_command", "Read a demo command",
    "read_demo_command index", read_demo_command),
    COMMAND_DEFINITION("disable_demo_commands", "Disable the demo command loop on future startups.",
     "disable_demo_commands", disable_demo_commands),
    COMMAND_DEFINITION("enable_demo_commands", "Enable the demo command loop on future startups.",
     "enable_demo_commands", enable_demo_commands),
    COMMAND_DEFINITION("demo_commands_enabled", "Check if demo commands are enabled.",
     "demo_commands_enabled", demo_commands_enabled),
#if defined(TEENSYDUINO)
    COMMAND_DEFINITION("nop", "No operation (does nothing)", "nop", nop_func),
#endif
    COMMAND_DEFINITION(nullptr, nullptr, nullptr, nullptr),
};
