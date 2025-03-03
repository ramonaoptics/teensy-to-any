/* A small demo to show how the teensy-to-any can be used to control 5 neopixel LEDs.
 *
 * Ramona Optics - 2025
 */
const char *teensy_to_any_startup_commands[] = {
    // Setup the LED as an indicator pin
    "neopixel_update_pin 19",
    "neopixel_update_length 5",

    // set color to orange to indicate that we aren't ready yet
    "neopixel_set_pixel_color 0 56 10 1 0",
    "neopixel_set_pixel_color 1 56 10 1 0",
    "neopixel_set_pixel_color 2 56 10 1 0",
    "neopixel_set_pixel_color 3 56 10 1 0",
    "neopixel_set_pixel_color 4 56 10 1 0",
    "neopixel_begin",
    "neopixel_show",
    nullptr,
};


#define _SLEEP_1000ms ("sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2", "sleep 0.2")
#define _SLEEP_200ms ("sleep 0.2")
#define _SLEEP_100ms ("sleep 0.1")
#define _SLEEP_50ms ("sleep 0.05")

#define _SLEEP_PATTERN _SLEEP_100ms

// Change the fan speed
const char *teensy_to_any_demo_commands[] = {
    "neopixel_set_pixel_color 0 0 0 0 40",
    "neopixel_set_pixel_color 1 40 0 0 0",
    "neopixel_set_pixel_color 2 0 40 0 0",
    "neopixel_set_pixel_color 3 0 0 40 0",
    "neopixel_set_pixel_color 4 40 40 40 40",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 30",
    "neopixel_set_pixel_color 1 30 0 0 0",
    "neopixel_set_pixel_color 2 0 30 0 0",
    "neopixel_set_pixel_color 3 0 0 30 0",
    "neopixel_set_pixel_color 4 30 30 30 30",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 20",
    "neopixel_set_pixel_color 1 20 0 0 0",
    "neopixel_set_pixel_color 2 0 20 0 0",
    "neopixel_set_pixel_color 3 0 0 20 0",
    "neopixel_set_pixel_color 4 20 20 20 20",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 10",
    "neopixel_set_pixel_color 1 10 0 0 0",
    "neopixel_set_pixel_color 2 0 10 0 0",
    "neopixel_set_pixel_color 3 0 0 10 0",
    "neopixel_set_pixel_color 4 10 10 10 10",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 5",
    "neopixel_set_pixel_color 1 5 0 0 0",
    "neopixel_set_pixel_color 2 0 5 0 0",
    "neopixel_set_pixel_color 3 0 0 5 0",
    "neopixel_set_pixel_color 4 5 5 5 5",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 4",
    "neopixel_set_pixel_color 1 4 0 0 0",
    "neopixel_set_pixel_color 2 0 4 0 0",
    "neopixel_set_pixel_color 3 0 0 4 0",
    "neopixel_set_pixel_color 4 4 4 4 4",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 3",
    "neopixel_set_pixel_color 1 3 0 0 0",
    "neopixel_set_pixel_color 2 0 3 0 0",
    "neopixel_set_pixel_color 3 0 0 3 0",
    "neopixel_set_pixel_color 4 3 3 3 3",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 2",
    "neopixel_set_pixel_color 1 2 0 0 0",
    "neopixel_set_pixel_color 2 0 2 0 0",
    "neopixel_set_pixel_color 3 0 0 2 0",
    "neopixel_set_pixel_color 4 2 2 2 2",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 1",
    "neopixel_set_pixel_color 1 1 0 0 0",
    "neopixel_set_pixel_color 2 0 1 0 0",
    "neopixel_set_pixel_color 3 0 0 1 0",
    "neopixel_set_pixel_color 4 1 1 1 1",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 00",
    "neopixel_set_pixel_color 1 00 0 0 0",
    "neopixel_set_pixel_color 2 0 00 0 0",
    "neopixel_set_pixel_color 3 0 0 00 0",
    "neopixel_set_pixel_color 4 00 00 00 00",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 1",
    "neopixel_set_pixel_color 1 1 0 0 0",
    "neopixel_set_pixel_color 2 0 1 0 0",
    "neopixel_set_pixel_color 3 0 0 1 0",
    "neopixel_set_pixel_color 4 1 1 1 1",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 2",
    "neopixel_set_pixel_color 1 2 0 0 0",
    "neopixel_set_pixel_color 2 0 2 0 0",
    "neopixel_set_pixel_color 3 0 0 2 0",
    "neopixel_set_pixel_color 4 2 2 2 2",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 3",
    "neopixel_set_pixel_color 1 3 0 0 0",
    "neopixel_set_pixel_color 2 0 3 0 0",
    "neopixel_set_pixel_color 3 0 0 3 0",
    "neopixel_set_pixel_color 4 3 3 3 3",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 4",
    "neopixel_set_pixel_color 1 4 0 0 0",
    "neopixel_set_pixel_color 2 0 4 0 0",
    "neopixel_set_pixel_color 3 0 0 4 0",
    "neopixel_set_pixel_color 4 4 4 4 4",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 5",
    "neopixel_set_pixel_color 1 5 0 0 0",
    "neopixel_set_pixel_color 2 0 5 0 0",
    "neopixel_set_pixel_color 3 0 0 5 0",
    "neopixel_set_pixel_color 4 5 5 5 5",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 10",
    "neopixel_set_pixel_color 1 10 0 0 0",
    "neopixel_set_pixel_color 2 0 10 0 0",
    "neopixel_set_pixel_color 3 0 0 10 0",
    "neopixel_set_pixel_color 4 10 10 10 10",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 20",
    "neopixel_set_pixel_color 1 20 0 0 0",
    "neopixel_set_pixel_color 2 0 20 0 0",
    "neopixel_set_pixel_color 3 0 0 20 0",
    "neopixel_set_pixel_color 4 20 20 20 20",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 30",
    "neopixel_set_pixel_color 1 30 0 0 0",
    "neopixel_set_pixel_color 2 0 30 0 0",
    "neopixel_set_pixel_color 3 0 0 30 0",
    "neopixel_set_pixel_color 4 30 30 30 30",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 40",
    "neopixel_set_pixel_color 1 40 0 0 0",
    "neopixel_set_pixel_color 2 0 40 0 0",
    "neopixel_set_pixel_color 3 0 0 40 0",
    "neopixel_set_pixel_color 4 40 40 40 40",
    "neopixel_show",
    _SLEEP_PATTERN,

    "neopixel_set_pixel_color 0 0 0 0 50",
    "neopixel_set_pixel_color 1 50 0 0 0",
    "neopixel_set_pixel_color 2 0 50 0 0",
    "neopixel_set_pixel_color 3 0 0 50 0",
    "neopixel_set_pixel_color 4 50 50 50 50",
    "neopixel_show",
    _SLEEP_PATTERN,
    nullptr,
};
