/* Setup the Teensy 4 to control a 4 pin PWM fan on pin 2.
 *
 * After bootup, the Teensy is ready to control the fan, however
 * the fan is set to off.
 *
 * Ramona Optics - 2024-2025
 */
const char *teensy_to_any_startup_commands[] = {
    // Setup the LED as an indicator pin
    "gpio_pin_mode 13 OUTPUT 1",
    // Setup pin 2 as the PWM control pin
    "gpio_pin_mode 2 OUTPUT 1",
    // Set the frequency to 25 kHz
    "analog_write_frequency 2 25000",
    // Also setup the LED to tell us what is going on.
    "analog_write_frequency 13 25000",
    "analog_write 2 0",
    "analog_write 13 0",
    nullptr,
};

// Change the fan speed
const char *teensy_to_any_demo_commands[] = {
    nullptr,
};
