/* A small demo to show how the teensy-to-any can be used to control 5 neopixel LEDs.
 *
 * Ramona Optics - 2025
 */
const char *teensy_to_any_startup_commands[] = {
    // Setup the LED as an indicator pin
    "fastled_add_leds NEOPIXEL 1 19 5",

    // set color to orange to indicate that we aren't ready yet
    // It takes about 2-3 seconds for the serial module to boot up
    // The teensy will be unresponsive during this time.
    "fastled_set_rgb 0 255 140 0",
    "fastled_set_rgb 1 255 140 0",
    "fastled_set_rgb 2 255 140 0",
    "fastled_set_rgb 3 255 140 0",
    "fastled_set_rgb 4 255 140 0",
    "fastled_show 30",
    nullptr,
};


#define _SLEEP_1000ms ("sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2")
#define _SLEEP_200ms ("sleep 0.2")
#define _SLEEP_100ms ("sleep 0.1")
#define _SLEEP_50ms ("sleep 0.05")

#define _SLEEP_PATTERN _SLEEP_100ms

// Change the fan speed
const char *teensy_to_any_demo_commands[] = {
    // Setup the color for the demo
    // This shouldn't significantly slowdown the pattern
    "fastled_set_rgb 0 40 40 40",
    "fastled_set_rgb 1 40 0 0",
    "fastled_set_rgb 2 0 40 0",
    "fastled_set_rgb 3 0 0 40",
    "fastled_set_rgb 4 40 40 40",

    "fastled_show 250", _SLEEP_PATTERN,
    "fastled_show 200", _SLEEP_PATTERN,
    "fastled_show 150", _SLEEP_PATTERN,
    "fastled_show 100", _SLEEP_PATTERN,
    "fastled_show 50", _SLEEP_PATTERN,
    "fastled_show 25", _SLEEP_PATTERN,
    "fastled_show 15", _SLEEP_PATTERN,
    "fastled_show 5", _SLEEP_PATTERN,
    "fastled_show 0", _SLEEP_PATTERN,
    "fastled_show 5", _SLEEP_PATTERN,
    "fastled_show 15", _SLEEP_PATTERN,
    "fastled_show 25", _SLEEP_PATTERN,
    "fastled_show 50", _SLEEP_PATTERN,
    "fastled_show 100", _SLEEP_PATTERN,
    "fastled_show 150", _SLEEP_PATTERN,
    "fastled_show 200", _SLEEP_PATTERN,
    nullptr,
};
