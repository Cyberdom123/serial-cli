#ifndef SERIAL_CLI_H
#define SERIAL_CLI_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
  SERIAL_CLI_COMMAND_MAX_ARGS = 5,
  SERIAL_CLI_COMMAND_MAX_ARG_LENGTH = 32,
  SERIAL_CLI_OUTPUT_BUFFER_SIZE = 128,
  SERIAL_CLI_INPUT_BUFFER_SIZE =
      ((SERIAL_CLI_COMMAND_MAX_ARGS * SERIAL_CLI_COMMAND_MAX_ARG_LENGTH) + SERIAL_CLI_COMMAND_MAX_ARGS),
};

// Forward declaration
typedef struct SerialCLI SerialCLI;

/**
 * Callback function to execute a command.
 *
 * @param argc The number of arguments.
 * @param argv The arguments.
 */
typedef void (*SerialCLI_Command)(SerialCLI *cli, int argc, const char **argv);

/**
 * Callback function to write a string to the serial interface.
 *
 * @param str The string to write.
 * @param len The length of the string.
 */
typedef void (*SerialCLI_Write)(const char *str, size_t len);

typedef enum {
  SERIAL_CLI_STATE_GETTING_INPUT,
  SERIAL_CLI_PREPROCESSING_INPUT,
  SERIAL_CLI_PROCESSING_COMMAND,
} SerialCLI_State;

typedef struct SerialCLI_CommandEntry {
  SerialCLI_Command command;           ///< The command function.
  const char *commandName;             ///< Name of the command.
  const char *commandDescription;      ///< Description of the command.
  struct SerialCLI_CommandEntry *next; ///< Pointer to the next command entry.
} SerialCLI_CommandEntry;

typedef struct SerialCLI {
  SerialCLI_CommandEntry *commands;       ///< Linked list of registered commands.
  SerialCLI_CommandEntry *currentCommand; ///< Pointer to the currently processed command.
  SerialCLI_Write write;                  ///< The write callback function.
  char *contextToken;                     ///< The context token for strtok_r.

  SerialCLI_State state; ///< The state of the SerialCLI.
  size_t charCount;      ///< The number of characters in the input buffer.
  int tokenCount;        ///< The number of extracted tokens.

  char inputBuffer[SERIAL_CLI_INPUT_BUFFER_SIZE + 1]; ///< The input buffer, +1 for the null terminator.
  char tokens[SERIAL_CLI_COMMAND_MAX_ARGS]
             [SERIAL_CLI_COMMAND_MAX_ARG_LENGTH + 1]; ///< Extracted tokens, +1 for the null terminator.
} SerialCLI;

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
 * Write a string to the SerialCLI output.
 *
 * @param cli The SerialCLI instance.
 * @param format The format string.
 * @param ... The arguments for the format string.
 */
void SerialCLI_WriteString(SerialCLI *cli, const char *format, ...);

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