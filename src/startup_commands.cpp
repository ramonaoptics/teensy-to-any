#include "startup_commands.hpp"
// Attribute weak allows one to override this in their own cpp file
// enabling customization of these startup_commands depending
// on the application
namespace teensy_to_any{
__attribute__((weak)) const char *startup_commands[] = {
    nullptr,
};

__attribute__((weak)) const char *demo_commands[] = {
  nullptr,
};

int len_startup_commands;
int len_demo_commands;

void setup_startup_and_demo_commands() {
  len_startup_commands = 0;
  for (int i = 0; startup_commands[i] != nullptr; i++) {
    len_startup_commands++;
  }
  len_demo_commands = 0;
  for (int i = 0; demo_commands[i] != nullptr; i++) {
    len_demo_commands++;
  }
}
}
