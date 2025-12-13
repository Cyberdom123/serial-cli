#ifndef SERIAL_CLI_INTERNAL_H_
#define SERIAL_CLI_INTERNAL_H_

#include "serial_cli.h"
#include <ctype.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function to get the arguments as an array of strings.
 *
 * @param cli The SerialCLI instance.
 * @param argv The array to fill with argument strings.
 */
static inline void SerialCLI_GetArgv(SerialCLI *cli, char *argv[]);

// Inline implementation below

static inline void SerialCLI_GetArgv(SerialCLI *cli, char *argv[]) {
  for (int i = 0; i < SERIAL_CLI_COMMAND_MAX_ARGS; ++i) {
    argv[i] = cli->tokens[i];
  }
  argv[SERIAL_CLI_COMMAND_MAX_ARGS] = NULL;
}

#ifdef __cplusplus
}
#endif

#endif // SERIAL_CLI_INTERNAL_H