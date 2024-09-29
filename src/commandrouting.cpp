#include "commandrouting.hpp"
#include <Arduino.h>
#include <errno.h>
#include <EEPROM.h>

#define LICENSE_TEXT                                                           \
  "teensytoany: hardware debugger based on the Teensy platform\n"              \
  "Copyright (C) 2019-2023  Ramona Optics, Inc.\n"                                  \
  "\n"                                                                         \
  "This program is free software: you can redistribute it and/or modify\n"     \
  "it under the terms of the GNU General Public License as published by\n"     \
  "the Free Software Foundation, either version 3 of the License, or\n"        \
  "(at your option) any later version.\n"                                      \
  "\n"                                                                         \
  "This program is distributed in the hope that it will be useful,\n"          \
  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"           \
  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"            \
  "GNU General Public License for more details.\n"                             \
  "\n"                                                                         \
  "You should have received a copy of the GNU General Public License\n"        \
  "along with this program.  If not, see "                                     \
  "<https://www.gnu.org/licenses/>.\n"

CommandRouter cmd;

int CommandRouter::init(const command_item_t *commands, int buffer_size,
                        int argv_max) {
  if (argv_max == 0) {
    return EINVAL;
  }
  if (buffer_size == 0) {
    return EINVAL;
  }

  argv = new const char *[argv_max];
  if (argv == nullptr)
    goto fail_argv_alloc;

  this->argv_max = argv_max;

  // TODO: do I need a + 1 here?
  buffer = new char[buffer_size + 1];
  if (buffer == nullptr)
    goto fail_buffer_alloc;
  this->buffer_size = buffer_size;

  command_list = commands;
  malloc_used = true;
  return 0;

  delete buffer;
  buffer = nullptr;
  this->buffer_size = 0;
fail_buffer_alloc:
  delete argv;
  argv = nullptr;
  this->argv_max = 0;
fail_argv_alloc:
  return ENOMEM;
}

int CommandRouter::init_no_malloc(const command_item_t *commands, int buffer_size,
                                  char *serial_buffer, int argv_max,
                                  const char **argv_buffer) {

  if (commands == nullptr) {
    return EINVAL;
  }
  if (buffer_size == 0) {
    return EINVAL;
  }
  if (argv_max == 0) {
    return EINVAL;
  }
  if (serial_buffer == nullptr) {
    return EINVAL;
  }
  if (argv_buffer == nullptr) {
    return EINVAL;
  }

  cleanup();

  this->buffer = serial_buffer;
  this->argv = argv_buffer;
  this->buffer_size = buffer_size;
  this->argv_max = argv_max;
  this->command_list = commands;
  this->malloc_used = false;
  return 0;
}

void CommandRouter::cleanup() {
  if (malloc_used) {
    if (buffer != nullptr) {
      delete buffer;
    }
    if (argv != nullptr) {
      delete argv;
    }
  }

  buffer = nullptr;
  buffer_size = 0;
  argv = nullptr;
  argv_max = 0;
  command_list = nullptr;
  malloc_used = false;
}

CommandRouter::~CommandRouter() {
  cleanup();
}

int CommandRouter::help(const char *command_name) {
  if (command_name == nullptr) {
    Serial.print(F("-----------------------------------\n"));
    Serial.print(F("Command List:\n"));
    Serial.print(F("-----------------------------------\n"));
  }
  for (int i = 0; command_list[i].name != nullptr; i++) {
    if (command_name == nullptr ||
        (strcmp(command_name, command_list[i].name) == 0)) {
      Serial.print(F("COMMAND: \n"));
      Serial.print(command_list[i].name);
      Serial.print("\n");
      Serial.print(F("SYNTAX:\n"));
      Serial.print(command_list[i].syntax);
      Serial.print("\n");
      Serial.print(F("DESCRIPTION:\n"));
      Serial.print(command_list[i].description);
      Serial.print("\n");
      if (command_name == nullptr) {
        Serial.print(F("-----------------------------------\n"));
      } else {
        return 0;
      }
    }
  }
  if (command_name == nullptr) {
    return 0;
  } else {
    snprintf(buffer, buffer_size, "%s not found", command_name);
    return EINVAL;
  }
}

int CommandRouter::route(int argc, const char **argv) {
  if (command_list == nullptr)
    return EINVAL;

  if (argc == 0)
    return EINVAL;

  if (argv[0] == nullptr)
    return EINVAL;
#if 0
  Serial.printf("Received command: %s", argv[0]);
#endif
  for (int i = 0; command_list[i].name != nullptr; i++) {
    if (strcmp(argv[0], command_list[i].name) == 0) {
      if (command_list[i].func != nullptr) {
        return command_list[i].func(this, argc, argv);
      } else {
        snprintf(buffer, buffer_size, "%s not found", argv[0]);
        return ENOSYS;
      }
    }
  }
  return ENOENT;
}

int command_license_func(CommandRouter *cmd, int argc, const char **argv) {
  if (argc != 1) {
    return EINVAL;
  }
  Serial.print(LICENSE_TEXT);
  return 0;
}

int command_help_func(CommandRouter *cmd, int argc, const char **argv) {
  if (argc == 1) {
    return cmd->help(nullptr);
  } else {
    return cmd->help(argv[1]);
  }
}

static void parseInputBuffer(char *input_buffer, int bytes_read, const char **argv, int &argc) {
  // This could likely be replaced by strtok_r, but I really failed
  // at using it on the teensy 4.0
  // And as such, I'm simply ignoring using it.
  int i = 0;
  argc = 0;
  while (i < bytes_read) {
    char c = input_buffer[i];
    if (c == '\0')
      break;
    if (input_buffer[i] == ' ') {
      input_buffer[i] = '\0';
      i++;
      continue;
    }
    argv[argc] = &input_buffer[i];
    argc++;
    while (i < bytes_read) {
      i += 1;
      c = input_buffer[i];
      if (c == ' ' || c == '\0')
        break;
    }
  }
}

int CommandRouter::processSerialStream() {
  int argc;
  int bytes_read = 0;
  int bytes_read_max = buffer_size - 1 - 1;
  int result;
  // The results will be placed in the
  // buffer starting at index 0 AFTER they have used the
  // parameter
  // So we place the input parametrers at index 1
  char *input_buffer = &this->buffer[1];
  this->buffer[0] = '\0'; // Null terminate the return string

  // TODO: is this limit correct?
  while (bytes_read < bytes_read_max) {
    int c;
    c = Serial.read();
    if (c == -1) {
      continue;
    }
    if (c == '\n' || c == '\r')
      break;
    if (c == '\b') {
      bytes_read = bytes_read == 0 ? 0 : bytes_read - 1;
      continue;
    }

    input_buffer[bytes_read] = (char)c;
    bytes_read++;
  }
  // Just flush the serial buffer, and terminate the command
  if (bytes_read == bytes_read_max) {
    result = ENOMEM;
    while (true) {
      int c;
      c = Serial.read();
      if (c == '\n' || c == '\r') {
        goto finish;
      }
    }
  }
  input_buffer[bytes_read] = '\0';
  if (bytes_read == 0) {
    return 0;
  }

  parseInputBuffer(input_buffer, bytes_read, argv, argc);

#if 0
  Serial.printf("argc == %d\n", argc);
  for (int i = 0; i < argc; i++) {
    Serial.printf("argv[%d] = %s\n", i, argv[i]);
  }
#endif
  result = route(argc, argv);

finish:
  // Print the return code
  Serial.print(result);

  // And any payload that exists
  if (buffer[0]) {
    Serial.print(" ");
    Serial.print(buffer);
  }
  Serial.print("\n");

  // Call send_now on success to ensure that the response is immediate
  // Otherwise, the serial buffer will wait for up to 5 ms
  // for the buffer to get full (it never will) before sending any data
  // https://www.pjrc.com/teensy/td_serial.html
  Serial.send_now();

  return result;
}

int CommandRouter::processEEPROMStream(int address, int length){
  int bytes_read;
  int result;
  while (length > 0) {
    result = processEEPROMSingleCommand(address, bytes_read);
    if (result != 0) {
      return result;
    }
    address += bytes_read;
    length -= bytes_read;

    if (bytes_read == 0) {
      break;
    }
  }
  return 0;
}

int CommandRouter::processEEPROMSingleCommand(int address, int &bytes_read) {
  int argc;
  int bytes_read_max = buffer_size - 1 - 1;
  int result;
  // The results will be placed in the
  // buffer starting at index 0 AFTER they have used the
  // parameter
  // So we place the input parametrers at index 1
  char *input_buffer = &this->buffer[1];
  this->buffer[0] = '\0';
  bytes_read = 0;

  // TODO: is this limit correct?
  while (bytes_read < bytes_read_max) {
    int c;
    c = EEPROM.read(address + bytes_read);
    if (c == -1) {
      continue;
    }
    if (c == '\n' || c == '\r')
      break;
    if (c == '\b') {
      bytes_read = bytes_read == 0 ? 0 : bytes_read - 1;
      continue;
    }

    input_buffer[bytes_read] = (char)c;
    bytes_read++;
  }
  if (bytes_read == bytes_read_max) {
    result = ENOMEM;
  }
  input_buffer[bytes_read] = '\0';
  if (bytes_read == 0) {
    return 0;
  }
  parseInputBuffer(input_buffer, bytes_read, argv, argc);
  result = route(argc, argv);
  return result;
}
