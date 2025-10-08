#ifndef SERIAL_CLI_INTERNAL_H
#define SERIAL_CLI_INTERNAL_H

#include "serial_cli.h"
#include <ctype.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static inline SerialCLI_CommandEntry *SerialCLI_GetCommand(SerialCLI *cli, const char *commandName);

/**
 * Function to get the arguments as an array of strings.
 *
 * @param cli The SerialCLI instance.
 * @param argv The array to fill with argument strings.
 */
static inline void SerialCLI_GetArgv(SerialCLI *cli, char *argv[]);

/**
 * Function to format the input by replacing whitespace characters with a single space.
 *
 * @param cli The SerialCLI instance.
 */
static inline void SerialCLI_FormatInput(SerialCLI *cli);

// Inline implementation below

static inline SerialCLI_CommandEntry *SerialCLI_GetCommand(SerialCLI *cli, const char *commandName) {
  SerialCLI_CommandEntry *current = cli->commands;
  while (current != NULL) {
    if (0 == strcmp(current->commandName, commandName)) {
      return current;
    }
    current = current->next;
  }
  return NULL;
}

static inline void SerialCLI_GetArgv(SerialCLI *cli, char *argv[]) {
  for (int i = 0; i < SERIAL_CLI_COMMAND_MAX_ARGS; ++i) {
    argv[i] = cli->tokens[i];
  }
  argv[SERIAL_CLI_COMMAND_MAX_ARGS] = NULL;
}

static inline void SerialCLI_FormatInput(SerialCLI *cli) {
  for (size_t i = 0; i < cli->charCount; i++) {
    if (isspace((unsigned char)cli->inputBuffer[i])) {
      cli->inputBuffer[i] = ' ';
    }
  }
}

#ifdef __cplusplus
}
#endif

#endif // SERIAL_CLI_INTERNAL_H