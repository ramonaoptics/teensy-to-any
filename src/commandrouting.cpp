#include "commandrouting.hpp"
#include <Arduino.h>
#include <cstring>
#include <errno.h>

CommandRouter cmd;

int CommandRouter::init(command_item_t *commands, int argv_max,
                        int buffer_size) {

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

int CommandRouter::init_no_malloc(command_item_t *commands, int argv_max,
                                  int buffer_size, char *serial_buffer,
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
  malloc_used = false;
  return 0;
}

void CommandRouter::cleanup() {
  if (malloc_used && buffer != nullptr) {
    delete buffer;
  }
  buffer = nullptr;
  buffer_size = 0;

  if (malloc_used && argv != nullptr) {
    delete argv;
  }
  argv = nullptr;
  argv_max = 0;
  command_list = nullptr;
  malloc_used = false;
}

CommandRouter::~CommandRouter() {
  cleanup();
}

int CommandRouter::help() {
  Serial.print(F("-----------------------------------\n"));
  Serial.print(F("Command List:\n"));
  Serial.print(F("-----------------------------------\n"));
  for (int i = 0; command_list[i].name != nullptr; i++) {
    Serial.print(F("COMMAND: \n"));
    Serial.print(command_list[i].name);
    Serial.print("\n");
    Serial.print(F("SYNTAX:\n"));
    Serial.print(command_list[i].syntax);
    Serial.print("\n");
    Serial.print(F("DESCRIPTION:\n"));
    Serial.print(command_list[i].description);
    Serial.print("\n");
    Serial.print(F("-----------------------------------\n"));
  }
  return 0;
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
        return ENOSYS;
      }
    }
  }
  return ENOENT;
}

int command_help_func(CommandRouter *cmd, int argc, const char **argv) {
  return cmd->help();
}

int CommandRouter::processSerialStream() {
  int argc;
  char *remaining_tokens;
  int bytes_read = 0;
  int result;
  int c;
  while (bytes_read < buffer_size - 1) {
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

    buffer[bytes_read] = (char)c;
    bytes_read++;
  }
  buffer[bytes_read] = '\0';
  if (bytes_read == 0) {
    return 0;
  }

  remaining_tokens = buffer;
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
  if (result != 0) {
    // TODO: convert errno to string???
    Serial.printf("Error: code %d. %s\n", result, std::strerror(result));
  }
  return result;
}
