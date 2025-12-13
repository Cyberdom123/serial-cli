#ifndef SERIAL_CLI_COMMAND_H_
#define SERIAL_CLI_COMMAND_H_

#include "serial_cli.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Function to get a command entry by its name.
 *
 * @param cli The SerialCLI instance.
 * @param commandName The name of the command to retrieve.
 * @return Pointer to the SerialCLI_CommandEntry if found, NULL otherwise.
 */
SerialCLI_CommandEntry *SerialCLI_GetCommandEntry(SerialCLI *cli, const char *commandName);

const char *SerialCLI_ResolvePartialCommand(SerialCLI *cli, const char *partialName);

#ifdef __cplusplus
}
#endif

#endif // SERIAL_CLI_COMMAND_H_