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

}
