#pragma once

#include <inttypes.h>
#include <stdio.h>

// Need to declare command router since it is used by the command_item struct;
class CommandRouter;

typedef struct command_item {
  const char *name;
  const char *description;
  const char *syntax;
  int (*func)(CommandRouter *cmd, int argc, const char **argv);
} command_item_t;

class CommandRouter {
public:
  int init(command_item_t *commands, int argv_max, int buffer_size);
  int init_no_malloc(command_item_t *commands, int argv_max, int buffer_size,
                     char *serial_buffer, const char **argv_buffer);
  int help();
  int processSerialStream();
  void cleanup();
  ~CommandRouter();

private:
  int route(int argc, const char **argv);

  // Serial command holders
  char *buffer = nullptr; // Allow for terminating null byte
  int buffer_size = 0;
  const char **argv;
  int argv_max = 0;
  bool malloc_used = false;
  command_item_t *command_list;
};

extern CommandRouter cmd;

int command_help_func(CommandRouter *cmd, int argc, const char **argv);
