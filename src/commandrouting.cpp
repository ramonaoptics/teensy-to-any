#include "commandrouting.hpp"
#include <Arduino.h>
#include <cstring>
#include <errno.h>

CommandRouter cmd;

int CommandRouter::init(command_item_t *commands, int buffer_size,
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

int CommandRouter::init_no_malloc(command_item_t *commands, int buffer_size,
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

int command_help_func(CommandRouter *cmd, int argc, const char **argv) {
  if (argc == 1) {
    return cmd->help(nullptr);
  } else {
    return cmd->help(argv[1]);
  }
}

int CommandRouter::processSerialStream() {
  int argc;
  char *remaining_tokens;
  int bytes_read = 0;
  int result;
  int c;
  char *input_buffer = &buffer[1];
  buffer[0] = '\0'; // Null terminate the return string

  while (bytes_read < buffer_size - 1 - 1) {
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
  input_buffer[bytes_read] = '\0';
  if (bytes_read == 0) {
    return 0;
  }

  remaining_tokens = input_buffer;
  for (argc = 0; argc < argv_max - 1; argc++) {
    if (remaining_tokens[0] == '\0') {
      break;
    }
    argv[argc] = strtok_r(remaining_tokens, " ", &remaining_tokens);
  }
#if 0
  Serial.printf("argc == %d\n", argc);
  for (int i = 0; i < argc; i++) {
    Serial.printf("argv[%d] = %s\n", i, argv[i]);
  }
#endif
  // Catchall for commands that are too large.
  if (remaining_tokens[0] != '\0') {
    argv[argc] = remaining_tokens;
    argc += 1;
  }
  result = route(argc, argv);

  // Print the return code
  Serial.print(result);

  // And any payload that exists
  if (buffer[0]) {
    Serial.print(" ");
    Serial.print(buffer);
  }
  Serial.print("\n");
  return result;
}
