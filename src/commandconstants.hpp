#pragma once

#include "commandrouting.hpp"

int info_func(CommandRouter *cmd, int argc, const char **argv);
int reboot_func(CommandRouter *cmd, int argc, const char **argv);
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

// PWM Support
int analog_write(CommandRouter *cmd, int argc, const char **argv);
int analog_write_frequency(CommandRouter *cmd, int argc, const char **argv);
int analog_write_resolution(CommandRouter *cmd, int argc, const char **argv);
int analog_pulse(CommandRouter *cmd, int argc, const char **argv);

// Analog IO
int analog_read(CommandRouter *cmd, int argc, const char **argv);

// SPI support
int spi_begin(CommandRouter *cmd, int argc, const char **argv);
int spi_end(CommandRouter *cmd, int argc, const char **argv);
int spi_set_mosi(CommandRouter *cmd, int argc, const char **argv);
int spi_set_miso(CommandRouter *cmd, int argc, const char **argv);
int spi_set_sck(CommandRouter *cmd, int argc, const char **argv);
int spi_settings(CommandRouter *cmd, int argc, const char **argv);
int spi_begin_transaction(CommandRouter *cmd, int argc, const char **argv);
int spi_end_transaction(CommandRouter *cmd, int argc, const char **argv);
int spi_transfer(CommandRouter *cmd, int argc, const char **argv);
int spi_read_byte(CommandRouter *cmd, int argc, const char **argv);
int spi_transfer_bulk(CommandRouter *cmd, int argc, const char **argv);
int spi_set_clock_divider(CommandRouter *cmd, int argc, const char **argv);

int register_read_uint8(CommandRouter *cmd, int argc, const char **argv);
int register_write_uint8(CommandRouter *cmd, int argc, const char **argv);
int register_read_uint16(CommandRouter *cmd, int argc, const char **argv);
int register_write_uint16(CommandRouter *cmd, int argc, const char **argv);
int register_read_uint32(CommandRouter *cmd, int argc, const char **argv);
int register_write_uint32(CommandRouter *cmd, int argc, const char **argv);

int eeprom_read_uint8(CommandRouter *cmd, int argc, const char **argv);
int eeprom_write_uint8(CommandRouter *cmd, int argc, const char **argv);

#ifdef TEENSYTOANY_USE_NEOPIXEL
int neopixel_init(CommandRouter *cmd, int argc, const char **argv);
int neopixel_begin(CommandRouter *cmd, int argc, const char **argv);
int neopixel_update_length(CommandRouter *cmd, int argc, const char **argv);
int neopixel_update_pin(CommandRouter *cmd, int argc, const char **argv);
int neopixel_update_type(CommandRouter *cmd, int argc, const char **argv);
int neopixel_show(CommandRouter *cmd, int argc, const char **argv);
int neopixel_set_pixel_color(CommandRouter *cmd, int argc, const char **argv);
#endif

// Mostly for debugging and startup scripts
int sleep_seconds(CommandRouter *cmd, int argc, const char **argv);
int startup_commands_available(CommandRouter *cmd, int argc, const char **argv);
int read_startup_command(CommandRouter *cmd, int argc, const char **argv);
int demo_commands_available(CommandRouter *cmd, int argc, const char **argv);
int read_demo_command(CommandRouter *cmd, int argc, const char **argv);
int disable_demo_commands(CommandRouter *cmd, int argc, const char **argv);
int enable_demo_commands(CommandRouter *cmd, int argc, const char **argv);
int demo_commands_available(CommandRouter *cmd, int argc, const char **argv);
int demo_commands_enabled(CommandRouter *cmd, int argc, const char **argv);

// Syntax is: {short command, description, syntax}
const command_item_t command_list[] = {
    {"?", "Display help info", "?", command_help_func},
    {"info", "Displays information about this TeensyToAny device", "about", info_func},
    {"reboot", "Runs setup routine again, for this device.", "reboot", reboot_func},
    {"mcu", "Displays information about the microcontroller board.", "mcu", mcu_func},
    {"serialnumber", "Displays the serial number of the board.", "serialnumber", serialnumber_func},
    {"license",
     "Display the license information for the source code running on the "
     "teensy",
     "license", command_license_func},
    {"version", "Display controller version number", "version", version_func},
    {"gpio_pin_mode",
     "Set the pinMode of a GPIO pin, 0 for INPUT, 1 for OUTPUT. "
     "Optionally, the value can be provided. If the value is provided "
     "a digitalWrite will be issued.",
     "gpio_pin_mode pin mode [value]", gpio_pin_mode},
    {"gpio_digital_write",
     "Set the output of a GPIO pin, 0 for LOW, 1 for HIGH",
     "gpio_digital_write pin value", gpio_digital_write},
    {"gpio_digital_read", "Read the value of a GPIO pin, 0 for LOW, 1 for HIGH",
     "gpio_digital_read pin", gpio_digital_read},
    {"gpio_digital_pulse",
     "Pulse the output of a GPIO pin for a given duration in seconds.",
     "gpio_digital_pulse pin value value_end duration_us", gpio_digital_pulse},
    {"i2c_init", "Initialize I2C Communication",
     "i2c_init [baudrate=100_000] [timeout_ms=200_000] [address_size=2] "
     "[address_msb_first=1]",
     i2c_init},
    {"i2c_reset", "Reset the I2C PORT in case of lockup.", "i2c_reset",
     i2c_reset},
    {"i2c_write_uint16", "Write a 16 bit number to the I2C device",
     "i2c_write_uint16 slave_address register_address data", i2c_write_uint16},
    {"i2c_read_uint16", "Read a 16 bit number from the I2C device",
     "i2c_read_uint16 slave_address register_address", i2c_read_uint16},
    {"i2c_write_uint8", "Write a 16 bit number to the I2C device",
     "i2c_write_uint8 slave_address register_address data", i2c_write_uint8},
    {"i2c_read_uint8", "Read a 16 bit number from the I2C device",
     "i2c_read_uint8 slave_address register_address", i2c_read_uint8},
    {"i2c_read_no_register_uint8",
     "Read a uint8_t from the I2C bus without specifying a register address.",
     "i2c_read_no_register_uint8 slave_address", i2c_read_no_register_uint8},
    {"i2c_write_no_register_uint8",
     "Write a uint8_t to the I2C bus without specifying a register address.",
     "i2c_write_no_register_uint8 slave_address data",
     i2c_write_no_register_uint8},
    {"i2c_write_payload",
     "Write up to 8 bytes to the I2C bus starting at a specified register "
     "address.",
     "i2c_write_payload slave_address register_address data num_bytes",
     i2c_write_payload},
    {"i2c_read_payload",
     "Read up to 8 bytes from the I2C bus starting at a specified 8 bit "
     "register address.",
     "i2c_read_payload slave_address register_address data num_bytes",
     i2c_read_payload},
    {"i2c_read_payload_no_register",
     "Read up to 16 bytes from the I2C bus without specifying a register "
     "address.",
     "i2c_read_payload_no_register slave_address data num_bytes",
     i2c_read_payload_no_register},
     {"i2c_read_payload_uint16",
      "Read up to 16 bytes from the I2C bus starting at a specified 16 bit "
      "register address.",
      "i2c_read_payload_uint16 slave_address register_address data num_bytes",
      i2c_read_payload_uint16},
    {"analog_write", "Write the duty cycle of the PWM",
     "analog_write pin dutycycle", analog_write},
    {"analog_write_frequency", "Write the frequency of the PWM",
     "analog_write_frequency pin frequency", analog_write_frequency},
    {"analog_write_resolution", "Write the resolution of the PWM timer",
     "analog_write_resolution bitdepth", analog_write_resolution},
    {"analog_pulse", "Write the duty cycle of the PWM for a given duration of time.",
     "analog_pulse pin dutycycle dutycycle_end duration", analog_pulse},
    {"analog_read", "Read the value of an analog pin, 0-255",
     "analog_read pin", analog_read},
    {"spi_begin", "SPI Begin", "spi_begin", spi_begin},
    {"spi_end", "SPI End", "spi_end", spi_begin},
    {"spi_set_mosi", "SPI set MOSI", "spi_set_mosi mosi", spi_set_mosi},
    {"spi_set_miso", "SPI set MISO ", " spi_set_miso miso", spi_set_miso},
    {"spi_set_sck", "SPI set SCK ", " spi_set_sck sck", spi_set_sck},
    {"spi_settings", "SPI set setting",
     "spi_settings frequency bitOrder dataMode", spi_settings},
    {"spi_begin_transaction", "spi_begin_transaction", "spi_begin_transaction",
     spi_begin_transaction},
    {"spi_end_transaction", "spi_end_transaction", "spi_end_transaction",
     spi_end_transaction},
    {"spi_transfer", "SPI Transfer 8bits of data", "spi_transfer data",
     spi_transfer},
    {"spi_read_byte", "SPI transfer register address and read a byte",
     "spi_read_byte data", spi_read_byte},
    {"spi_transfer_bulk", "SPI transfer multiple sets of 8 bits of data",
     "spi_transfer_bulk data[0] data[1] data[2] [...]", spi_transfer_bulk},
    {"register_read_uint8", "Read an arbitrary hardware register.",
     "register_read_uint8 address", register_read_uint8},
    {"register_write_uint8", "Write to an arbitrary hardware register.",
     "register_write_uint8 address data", register_write_uint8},
    {"register_read_uint16", "Read an arbitrary hardware register.",
     "register_read_uint16 address", register_read_uint16},
    {"register_write_uint16", "Write to an arbitrary hardware register.",
     "register_write_uint16 address data", register_write_uint16},
    {"register_read_uint32", "Read an arbitrary hardware register.",
     "register_read_uint32 address", register_read_uint32},
    {"register_write_uint32", "Write to an arbitrary hardware register.",
     "register_write_uint32 address data", register_write_uint32},
    {"eeprom_read_uint8", "Read data from a given EEPROM address.",
     "eeprom_read_uint8 address", eeprom_read_uint8},
    {"eeprom_write_uint8", "Write to an EEPROM address.",
     "eeprom_write_uint8 address data", eeprom_write_uint8},
#ifdef TEENSYTOANY_USE_NEOPIXEL
    {"neopixel_init", "Initialize the neopixel library",
     "neopixel_init num_pixels pin type", neopixel_init},
    {"neopixel_begin", "Begin the neopixel buffer",
     "neopixel_begin", neopixel_begin},
    {"neopixel_update_length", "Update the length of the neopixel buffer",
     "neopixel_update_length num_pixels", neopixel_update_length},
    {"neopixel_update_pin", "Update the pin of the neopixel buffer",
     "neopixel_update_pin pin", neopixel_update_pin},
    {"neopixel_update_type", "Update the type of the neopixel buffer",
     "neopixel_update_type type", neopixel_update_type},
    {"neopixel_show", "Show the current neopixel buffer",
     "neopixel_show", neopixel_show},
    {"neopixel_set_pixel_color", "Set the color of a pixel in the neopixel buffer",
     "neopixel_set_pixel_color pixel red green blue", neopixel_set_pixel_color},
#endif
    {"sleep", "Sleep (and block) for the desired duration",
     "sleep duration", sleep_seconds},
    {"startup_commands_available", "Number of startup commands available",
     "startup_commands_available", startup_commands_available},
    {"read_startup_command", "Read a startup command",
     "read_startup_command index", read_startup_command},
    {"demo_commands_available", "Number of demo commands available",
     "demo_commands_available", demo_commands_available},
    {"read_demo_command", "Read a demo command",
    "read_demo_command index", read_demo_command},
    {"disable_demo_commands", "Disable the demo command loop on future startups.",
     "disable_demo_commands", disable_demo_commands},
    {"enable_demo_commands", "Enable the demo command loop on future startups.",
     "enable_demo_commands", enable_demo_commands},
    {"demo_commands_enabled", "Check if demo commands are enabled.",
     "demo_commands_enabled", demo_commands_enabled},
    {nullptr, nullptr, nullptr, nullptr},
};
