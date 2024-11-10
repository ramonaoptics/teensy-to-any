/* A small demo to show how the teensy-to-any can be used to control 4 pin fans.
 * It is assumed that the fan PWM pin is connected to pin 2
 * and that an indicator LED is connected to Pin 13 (this is the default board)
 *
 * The demo code will run the fan at
 *  * 100%
 *  * 75%
 *  * 50%
 *  * 25%
 *  * 0%
 * For 3 seconds at a time and then continue in an infinite loop.
 * The LED can be used as a visual guide of the expected speed of the fan.
 *
 * Not all fan will be able to operate at 25% duty cyle.
 *
 * Ramona Optics - 2024
 */
namespace teensy_to_any {
const char *startup_commands[] = {
    // Setup the LED as an indicator pin
    "gpio_pin_mode 13 OUTPUT 1",
    // Setup pin 2 as the PWM control pin
    "gpio_pin_mode 2 OUTPUT 1",
    // Set the frequency to 25 kHz
    "analog_write_frequency 2 25000",
    // Also setup the LED to tell us what is going on.
    "analog_write_frequency 13 25000",
    nullptr,
};

// Change the fan speed
const char *demo_commands[] = {
    "analog_write 2 255",
    "analog_write 13 255",
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms

    "analog_write 2 192",
     // Brightness is a strange thing, and does not
     // decrease so linearly according to human perception
    "analog_write 13 128",
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms

    "analog_write 2 128",
    "analog_write 13 30",
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms

    "analog_write 2 64",
    "analog_write 13 10",
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms

    "analog_write 2 0",
    "analog_write 13 0",
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", // 1000 ms
    nullptr,
};
}
