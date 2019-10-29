#pragma once

#include "commandrouting.hpp"

int command_help_func(CommandRouter *cmd, int argc, char **argv);
int help_func(CommandRouter *cmd, int argc, char **argv);
int info_func(CommandRouter *cmd, int argc, char **argv);
int reboot_func(CommandRouter *cmd, int argc, char **argv);
int version_func(CommandRouter *cmd, int argc, char **argv);

// Syntax is: {short command, long command, description, syntax}
command_item_t *command_list = {
    {"?", "help", "Display help info", "?", command_help_func},
    {"info", "about", "Displays information about this LED Array", "about",
     info_func},
    {"reboot", "reset", "Runs setup routine again, for resetting LED array",
     "reboot", reboot_func},
    {"ver", "version", "Display controller version number", "version",
     version_func},
    {nullptr, nullptr, nullptr, nullptr, nullptr}};
