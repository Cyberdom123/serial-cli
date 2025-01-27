#ifndef SERIAL_CLI_H
#define SERIAL_CLI_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SERIAL_CLI_COMMANDS_MAX 10
#define SERIAL_CLI_COMMAND_BUFFER_SIZE 128
#define SERIAL_CLI_COMMAND_MAX_ARGS 10
#define SERIAL_CLI_COMMAND_MAX_ARG_LENGTH 32

#define SERIAL_CLI_COMMAND_PROMPT "> "
#define SERIAL_CLI_COMMAND_HELP "help"

typedef void (*SERIAL_CLI_Command)(int argc, char **argv);

typedef struct {
  SERIAL_CLI_Command command;
  const char *command_name;
  const char *command_description;
} SERIAL_CLI_CommandEntry;

typedef struct {
  char inputBuffer[SERIAL_CLI_COMMAND_BUFFER_SIZE];
  SERIAL_CLI_CommandEntry commands[SERIAL_CLI_COMMANDS_MAX];
  size_t bufferIndex;
  size_t commandsCount;
} SERIAL_CLI;

/**
 * Write a string to the CLI output.
 * 
 * @param str The string to write.
 * @param len The length of the string.
 * 
 * @note This function is weakly linked and can be overridden by the user. 
 */
void cli_write(const char *str, size_t len) __attribute__((weak));

/**
 * Read a string from the CLI input.
 * 
 * @param str The buffer to read the string into.
 * @param len The length of the buffer.
 * 
 * @note This function is weakly linked and can be overridden by the user. 
 */
void cli_read(char *str, size_t len) __attribute__((weak));

bool SERIAL_CLI_Init(SERIAL_CLI *cli);

void SERIAL_CLI_Deinit(SERIAL_CLI *cli);

bool SERIAL_CLI_RegisterCommand(SERIAL_CLI *cli, SERIAL_CLI_CommandEntry *command);

void SERIAL_CLI_Process(SERIAL_CLI *cli);

#ifdef __cplusplus
}
#endif

#endif // SERIAL_CLI_H