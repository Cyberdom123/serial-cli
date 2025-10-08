#include "serial_cli.h"
#include "serial_cli_internal.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

enum {
  CARRIAGE_RETURN = '\r', // ASCII CR character
  BACKSPACE = 127,        // ASCII DEL character
};

static void resetCLI(SerialCLI *cli) {
  memset(cli->inputBuffer, 0, SERIAL_CLI_INPUT_BUFFER_SIZE);
  memset(cli->tokens, 0, sizeof(cli->tokens));

  cli->charCount = 0;
  cli->tokenCount = 0;
  cli->contextToken = cli->inputBuffer;
  cli->currentCommand = NULL;
  cli->state = SERIAL_CLI_STATE_GETTING_INPUT;

  SerialCLI_WriteString(cli, "\r\n%s", ">> ");
}

static void interpretCommand(SerialCLI *cli) {
  char *token = strtok_r(cli->contextToken, " ", &cli->contextToken);

  if (NULL != token) {
    //  Store the token
    bool isMaximumReached = (cli->tokenCount == SERIAL_CLI_COMMAND_MAX_ARGS);
    bool isArgTooLong = (strlen(token) > SERIAL_CLI_COMMAND_MAX_ARG_LENGTH);
    if (isMaximumReached || isArgTooLong) {
      resetCLI(cli);
      return;
    }

    strncpy(cli->tokens[cli->tokenCount], token, sizeof(cli->tokens[cli->tokenCount]) - 1);
    ++cli->tokenCount;

    // Find the command index, first always should be the command name
    if (1 == cli->tokenCount) {
      cli->currentCommand = SerialCLI_GetCommand(cli, token);
    }
    return;
  }

  bool isCommandValid = (NULL != cli->currentCommand);
  if (isCommandValid) {
    char *toWrite = "\r\n";
    cli->write(toWrite, strlen(toWrite));

    char *argv[SERIAL_CLI_COMMAND_MAX_ARGS + 1] = {0};
    SerialCLI_GetArgv(cli, argv);
    cli->currentCommand->command(cli, cli->tokenCount, (const char **)argv);
  }

  resetCLI(cli);
}

static void helpCommand(SerialCLI *cli, int argc, const char **argv) {
  (void)argv;

  if (argc > 1) {
    SerialCLI_WriteString(cli, "Usage: help\r\n");
    return;
  }

  SerialCLI_WriteString(cli, "Available commands:\r\n");

  SerialCLI_CommandEntry *current = cli->commands;
  while (current != NULL) {
    if (NULL != current->commandName) {
      SerialCLI_WriteString(cli, "  %s - ", current->commandName);
    }
    if (NULL != current->commandDescription) {
      SerialCLI_WriteString(cli, "%s\r\n", current->commandDescription);
    }

    current = current->next;
  }
}

bool SerialCLI_Init(SerialCLI *cli, SerialCLI_Write write) {
  if (NULL == cli || NULL == write) {
    return false;
  }

  cli->write = write;
  cli->commands = NULL;
  cli->currentCommand = NULL;
  resetCLI(cli);

  static SerialCLI_CommandEntry helpEntry;
  helpEntry.command = helpCommand;
  helpEntry.commandName = "help";
  helpEntry.commandDescription = "Prints all available commands";
  SerialCLI_RegisterCommand(cli, &helpEntry);
  return true;
}

void SerialCLI_Deinit(SerialCLI *cli) {
  if (NULL == cli) {
    return;
  }

  resetCLI(cli);
}

void SerialCLI_Read(SerialCLI *cli, const char *str, size_t length) {
  if (NULL == cli || (NULL == str) || (0 >= length) || (SERIAL_CLI_OUTPUT_BUFFER_SIZE < length)) {
    return;
  }

  char output[SERIAL_CLI_OUTPUT_BUFFER_SIZE] = {0};
  size_t outputLen = 0;

  for (size_t i = 0; i < length; ++i) {
    if (SERIAL_CLI_INPUT_BUFFER_SIZE == cli->charCount) {
      resetCLI(cli);
      return;
    }

    if (CARRIAGE_RETURN == str[i]) {
      cli->state = SERIAL_CLI_PREPROCESSING_INPUT;
      return;
    }

    if (BACKSPACE == str[i]) {
      if (cli->charCount == 0) {
        break; // Nothing to delete
      }

      // Remove the last character from the input buffer
      cli->inputBuffer[cli->charCount - 1] = '\0';
      cli->charCount--;

      // Add backspace sequence to output for visual feedback
      const char *deleteSequence = "\b \b";
      if ((outputLen + strlen(deleteSequence) + 1) < sizeof(output)) {
        memcpy(output + outputLen, deleteSequence, strlen(deleteSequence));
        outputLen += strlen(deleteSequence);
      }
      break;
    }

    output[outputLen] = str[i];
    outputLen++;
    cli->inputBuffer[cli->charCount] = str[i];
    cli->charCount++;
  }

  // Write the input back to the serial interface
  if (outputLen > 0) {
    cli->write(output, outputLen);
  }
}

void SerialCLI_WriteString(SerialCLI *cli, const char *format, ...) {
  if (NULL == format || NULL == cli) {
    return;
  }

  char buffer[SERIAL_CLI_OUTPUT_BUFFER_SIZE];
  va_list arg;
  va_start(arg, format);
  vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  cli->write(buffer, strlen(buffer));
}

bool SerialCLI_RegisterCommand(SerialCLI *cli, SerialCLI_CommandEntry *command) {
  if ((NULL == cli) || (NULL == command) || (NULL == command->commandName) || (NULL == command->command)) {
    return false;
  }

  if (NULL == cli->commands) {
    command->next = NULL;
    cli->commands = command;
    return true;
  }

  SerialCLI_CommandEntry *current = cli->commands;
  // Check if command is already registered
  if (current == command) {
    return false;
  }

  while (current->next != NULL) {
    // Check if command is already registered
    if (current->next == command) {
      return false;
    }
    current = current->next;
  }

  command->next = NULL;
  current->next = command;
  return true;
}

void SerialCLI_Process(SerialCLI *cli) {
  if (NULL == cli) {
    return;
  }

  switch (cli->state) {
  case SERIAL_CLI_STATE_GETTING_INPUT:
    return;
  case SERIAL_CLI_PREPROCESSING_INPUT:
    SerialCLI_FormatInput(cli);
    cli->state = SERIAL_CLI_PROCESSING_COMMAND;
    return;
  case SERIAL_CLI_PROCESSING_COMMAND:
    interpretCommand(cli);
    return;
  }
}
