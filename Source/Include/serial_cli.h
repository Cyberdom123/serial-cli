#ifndef SERIAL_CLI_H
#define SERIAL_CLI_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  SERIAL_CLI_COMMANDS_MAX = 10,
  SERIAL_CLI_COMMAND_BUFFER_SIZE = 128,
  SERIAL_CLI_COMMAND_MAX_ARGS = 10,
  SERIAL_CLI_COMMAND_MAX_ARG_LENGTH = 32,
};

#define SERIAL_CLI_COMMAND_PROMPT "> "
#define SERIAL_CLI_COMMAND_HELP "help"

typedef void (*SerialCLI_Command)(int argc, const char **argv);

typedef struct {
  SerialCLI_Command command;
  const char *command_name;
  const char *command_description;
} SerialCLI_CommandEntry;

typedef struct {
  char inputBuffer[SERIAL_CLI_COMMAND_BUFFER_SIZE];
  SerialCLI_CommandEntry commands[SERIAL_CLI_COMMANDS_MAX];
  size_t bufferIndex;
  size_t commandsCount;
} SerialCLI;

/**
 * Callback function to write a string to the serial interface.
 *
 * @param str The string to write.
 * @param len The length of the string.
 */
typedef void (*SerialCLI_Write)(const char *str, size_t len);

/**
 * Function to read a string from the serial interface.
 *
 * @param cli The SerialCLI instance.
 * @param str The buffer to read the string into.
 * @param len The length of the buffer.
 */
void SerialCLI_Read(SerialCLI *cli, const char *str, size_t len);

/**
 * Initialize the SerialCLI.
 *
 * @param cli The SerialCLI instance.
 * @param write The write callback function.
 *
 * @return true if the initialization was successful, false otherwise.
 */
bool SerialCLI_Init(SerialCLI *cli, SerialCLI_Write write);

/**
 * Deinitialize the SerialCLI.
 *
 * @param cli The SerialCLI instance.
 */
void SerialCLI_Deinit(SerialCLI *cli);

/**
 * Register a command with the SerialCLI.
 *
 * @param cli The SerialCLI instance.
 * @param command The command to register.
 */
bool SerialCLI_RegisterCommand(SerialCLI *cli, SerialCLI_CommandEntry *command);

/**
 * Process the SerialCLI.
 *
 * @param cli The SerialCLI instance.
 */
void SerialCLI_Process(SerialCLI *cli);

#ifdef __cplusplus
}
#endif

#endif // SERIAL_CLI_H