#include "startup_commands.hpp"
// Attribute weak allows one to override this in their own cpp file
// enabling customization of these startup_commands depending
// on the application
__attribute__((weak)) const char *teensy_to_any_startup_commands[] = {
    nullptr,
};

__attribute__((weak)) const char *teensy_to_any_demo_commands[] = {
  nullptr,
};
