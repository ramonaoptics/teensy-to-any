#pragma once

#include "commandrouting.hpp"

int info_func(CommandRouter *cmd, int argc, const char **argv);
int reboot_func(CommandRouter *cmd, int argc, const char **argv);
int version_func(CommandRouter *cmd, int argc, const char **argv);
int command_license_func(CommandRouter *cmd, int argc, const char **argv);

// Digital GPIO
int gpio_pin_mode(CommandRouter *cmd, int argc, const char **argv);
int gpio_digital_write(CommandRouter *cmd, int argc, const char **argv);
int gpio_digital_read(CommandRouter *cmd, int argc, const char **argv);

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

// PWM Support
int analog_write(CommandRouter *cmd, int argc, const char **argv);
int analog_write_frequency(CommandRouter *cmd, int argc, const char **argv);
int analog_write_resolution(CommandRouter *cmd, int argc, const char **argv);

// Syntax is: {short command, description, syntax}
command_item_t command_list[] = {
    {"?", "Display help info", "?", command_help_func},
    {"info", "Displays information about this LED Array", "about", info_func},
    {"reboot", "Runs setup routine again, for resetting LED array", "reboot",
     reboot_func},
    {"license",
     "Display the license information for the source code running on the "
     "teensy",
     "license", command_license_func},
    {"version", "Display controller version number", "version", version_func},
    {"gpio_pin_mode",
     "Set the pinMode of a GPIO pin, 0 for INPUT, 1 for OUTPUT",
     "gpio_pin_mode pin mode", gpio_pin_mode},
    {"gpio_digital_write",
     "Set the output of a GPIO pin, 0 for LOW, 1 for HIGH",
     "gpio_digital_write pin value", gpio_digital_write},
    {"gpio_digital_read", "Read the value of a GPIO pin, 0 for LOW, 1 for HIGH",
     "gpio_digital_read pin value", gpio_digital_read},
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
    {"analog_write", "Write the duty cycle of the PWM",
     "analog_write pin dutycycle", analog_write},
    {"analog_write_frequency", "Write the frequency of the PWM",
     "analog_write_frequency pin frequency", analog_write_frequency},
    {"analog_write_resolution", "Write the resolution of the PWM timer",
     "analog_write_resolution bitdepth", analog_write_resolution},
    {nullptr, nullptr, nullptr, nullptr}};
