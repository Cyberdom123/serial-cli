#include "serial_cli_commands.h"

#include <stddef.h>
#include <string.h>

SerialCLI_CommandEntry *SerialCLI_GetCommandEntry(SerialCLI *cli, const char *commandName) {
  SerialCLI_CommandEntry *current = &cli->commands;
  while (NULL != current) {
    if (0 == strncmp(current->commandName, commandName, SERIAL_CLI_COMMAND_MAX_ARG_LENGTH)) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

const char *SerialCLI_ResolvePartialCommand(SerialCLI *cli, const char *partialName) {
  SerialCLI_CommandEntry *current = &cli->commands;
  size_t matchCount = 0;

  const char *command = NULL;
  size_t partialLen = strlen(partialName);
  while (NULL != current) {
    if (NULL != current->commandName) {
      if (partialLen >= strlen(current->commandName)) {
        current = current->next;
        continue;
      }

      if (0 == strncmp(partialName, current->commandName, partialLen)) {
        command = current->commandName;
        ++matchCount;
      }
    }
    current = current->next;
  }

  if (1 == matchCount) {
    return command;
  }
  return NULL;
}