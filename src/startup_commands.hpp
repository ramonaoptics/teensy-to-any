#pragma once

namespace teensy_to_any {
extern int len_startup_commands;
extern int len_demo_commands;
// These commands can be defined by the user
// see the startup_commands.cpp file for an example
extern const char *startup_commands[];
extern const char *demo_commands[];

void setup_startup_and_demo_commands();
} // namespace teensy_to_any