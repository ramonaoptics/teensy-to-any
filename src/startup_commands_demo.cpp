/* Demo file for how to use the startup commands
 *
 * This kind of file is meant to be included in your own project
 * We define a variable called startup_commands and demo_commands
 * in the teensy_to_any namespace. This is then used by the
 * startup_commands.cpp file to setup the startup commands that will
 * be run once when the TeensyToAny device is powered on before the
 * serial port is opened.
 *
 * The demo commands are similar to the startup commands, but they
 * are run in a loop indefinitely.
 * This is useful for running a demo on a device that is not attached
 * to a computer.
 *
 * The demo commands can be disabled by writing a value other than 0xFF to
 * a specified location in the EEPROM.
 */
#ifdef TEENSY_TO_ANY_STARTUP_COMMANDS_DEMO
namespace teensy_to_any {
const char *startup_commands[] = {
    "gpio_pin_mode 13 OUTPUT 1",
    "gpio_digital_pulse 13 0 1 200E-3",
    "gpio_digital_pulse 13 1 0 200E-3",
    "gpio_digital_pulse 13 0 1 100E-3",
    "gpio_digital_pulse 13 1 0 100E-3",
    "gpio_digital_pulse 13 0 1 100E-3",
    "gpio_digital_pulse 13 1 0 100E-3",
    "gpio_digital_pulse 13 0 1 100E-3",
    "gpio_digital_pulse 13 1 0 100E-3",
    "gpio_digital_pulse 13 0 1 200E-3",
    "gpio_digital_pulse 13 1 0 200E-3",
    nullptr,
};

const char *demo_commands[] = {
    "gpio_digital_pulse 13 0 1 50E-3",
    "gpio_digital_pulse 13 1 0 50E-3",
    nullptr,
};
}
#endif