#include "serial_cli.h"

#include <assert.h>
#include <string.h>

static size_t SerialCLI_GetCommandIndex(SerialCLI *cli, const char *command) {
  assert(NULL != cli);
  assert(NULL != command);

  if (NULL == cli || NULL == command) {
    return SERIAL_CLI_COMMANDS_MAX;
  }

  for (size_t i = 0; i < cli->commandsCount; i++) {
    if (0 == strcmp(cli->commands[i].command_name, command)) {
      return i;
    }
  }
  return SERIAL_CLI_COMMANDS_MAX;
}

void SerialCLI_Read(SerialCLI *cli, const char *str, size_t len) {
  assert(NULL != str);
  assert(0 < len);

  if (NULL == str || 0 >= len) {
    return;
  }

  // Add string to input buffer
  for (size_t i = 0; i < len; i++) {
    if (cli->bufferIndex < SERIAL_CLI_COMMAND_BUFFER_SIZE) {
      cli->inputBuffer[cli->bufferIndex] = str[i];
      cli->bufferIndex++;
    }
  }
}

bool SerialCLI_Init(SerialCLI *cli, SerialCLI_Write write) {
  assert(NULL != cli);
  assert(NULL != write);

  if (NULL == cli) {
    return false;
  }
  return true;
}

void SerialCLI_Deinit(SerialCLI *cli) {
  assert(NULL != cli);

  if (NULL == cli) {
    return;
  }
  cli->commandsCount = 0;
}

bool SerialCLI_RegisterCommand(SerialCLI *cli, SerialCLI_CommandEntry *command) {
  assert(NULL != cli);
  assert(NULL != command);
  assert(NULL != command->command_name);
  assert(NULL != command->command);

  if (NULL == cli || NULL == command || NULL == command->command_name || NULL == command->command) {
    return false;
  }
  memcpy(&cli->commands[cli->commandsCount], command, sizeof(SerialCLI_CommandEntry));
  cli->commandsCount++;
  return true;
}

void SerialCLI_Process(SerialCLI *cli) {
  assert(NULL != cli);
  if (NULL == cli) {
    return;
  }

  // Check if enter key was pressed
  if ('\r' != cli->inputBuffer[cli->bufferIndex - 1]) {
    return;
  }

  for (size_t i = 0; i < cli->commandsCount; i++) {
    SerialCLI_CommandEntry *command = &cli->commands[i];
    size_t command_len = strlen(command->command_name);

    if ((cli->bufferIndex - 1) == command_len) {
      size_t index = SerialCLI_GetCommandIndex(cli, command->command_name);
      if (SERIAL_CLI_COMMANDS_MAX == index) {
        // Command not found
        continue;
      }
      
      //TODO: Get Argc and Argv
      const char *argv[] = {"test"};
      cli->commands[index].command(1, argv);
      break;
    }
  }
  // Clear the buffer after executing the command
  cli->bufferIndex = 0;
  memset(cli->inputBuffer, 0, SERIAL_CLI_COMMAND_BUFFER_SIZE);
}