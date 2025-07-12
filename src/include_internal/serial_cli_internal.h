#ifndef SERIAL_CLI_INTERNAL_H
#define SERIAL_CLI_INTERNAL_H

#include "serial_cli.h"
#include <string.h>
#include <ctype.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function to get the command index based on the command name.
 * 
 * @param cli The SerialCLI instance.
 * @param string The command name to search for.
 * 
 * @return The index of the command if found, otherwise SERIAL_CLI_COMMANDS_MAX.
 */
static inline size_t SerialCLI_GetCommandIndex(SerialCLI *cli, const char *string);

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

static inline size_t SerialCLI_GetCommandIndex(SerialCLI *cli, const char *string) {
  if (strlen(string) == 0 || strlen(string) > SERIAL_CLI_COMMAND_MAX_ARG_LENGTH) {
    return SERIAL_CLI_COMMANDS_MAX;
  }

  for (size_t i = 0; i < cli->commandsCount; ++i) {
    if (NULL != cli->commands[i].commandName) {
      bool isNameMatch = (0 == strncmp(cli->commands[i].commandName, string, strlen(string)));
      if (isNameMatch) {
        return i;
      }
    }
  }

  return SERIAL_CLI_COMMANDS_MAX;
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