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
