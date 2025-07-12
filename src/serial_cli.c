#include "serial_cli.h"
#include "serial_cli_internal.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

enum {
  CARRIAGE_RETURN = '\r',
  BACKSPACE = 127,
};

static void resetCLI(SerialCLI *cli) {
  memset(cli->inputBuffer, 0, SERIAL_CLI_INPUT_BUFFER_SIZE);
  memset(cli->tokens, 0, sizeof(cli->tokens));

  cli->charCount = 0;
  cli->tokenCount = 0;
  cli->commandIndex = SERIAL_CLI_COMMANDS_MAX;
  cli->contextToken = cli->inputBuffer;
  cli->state = SERIAL_CLI_STATE_GETTING_INPUT;
  
  SerialCLI_WriteString(cli, "\r\n%s", SERIAL_CLI_COMMAND_PROMPT);
}

static void interpretCommand(SerialCLI *cli) {
  char *token = strtok_r(cli->contextToken, " ", &cli->contextToken);

  if (NULL != token) {
    //  Store the token
    if (cli->tokenCount == SERIAL_CLI_COMMAND_MAX_ARGS || strlen(token) > SERIAL_CLI_COMMAND_MAX_ARG_LENGTH) {
      resetCLI(cli);
      return;
    }

    strncpy(cli->tokens[cli->tokenCount], token, sizeof(cli->tokens[cli->tokenCount]) - 1);
    ++cli->tokenCount;

    // Find the command index, first always should be the command name
    if (1 == cli->tokenCount) {
      cli->commandIndex = SerialCLI_GetCommandIndex(cli, token);
    }
  } else {
    bool isCommandIndexValid = (SERIAL_CLI_COMMANDS_MAX != cli->commandIndex);
    if (isCommandIndexValid) {
      char *argv[SERIAL_CLI_COMMAND_MAX_ARGS + 1] = {0};
      SerialCLI_GetArgv(cli, argv);
      char *toWrite = "\r\n";
      cli->write(toWrite, strlen(toWrite)); 
      cli->commands[cli->commandIndex].command(cli, cli->tokenCount, (const char **)argv);
    }

    resetCLI(cli);
  }
}

static void helpCommand(SerialCLI *cli, int argc, const char **argv) {
  (void)argv;

  if (argc > 1) {
    SerialCLI_WriteString(cli, "Usage: help\r\n");
    return;
  }

  SerialCLI_WriteString(cli, "Available commands:\r\n");
  for (size_t i = 0; i < cli->commandsCount; ++i) {
    if (NULL != cli->commands[i].commandName) {
      SerialCLI_WriteString(cli, "  %s - ", cli->commands[i].commandName);
    }
    if (NULL != cli->commands[i].commandDescription) {
      SerialCLI_WriteString(cli, "%s\r\n", cli->commands[i].commandDescription);
    }
  }
}

bool SerialCLI_Init(SerialCLI *cli, SerialCLI_Write write) {
  if (NULL == cli || NULL == write) {
    return false;
  }
  cli->write = write;
  cli->commandsCount = 0;
  resetCLI(cli);

  SerialCLI_CommandEntry helpEntry;
  helpEntry.command = helpCommand;
  helpEntry.commandName = "help";
  helpEntry.commandDescription = "Prints the available commands.";
  SerialCLI_RegisterCommand(cli, &helpEntry);
  return true;
}

void SerialCLI_Deinit(SerialCLI *cli) {
  if (NULL == cli) {
    return;
  }
  resetCLI(cli);
  memset(cli->commands, 0, sizeof(cli->commands));
  cli->commandsCount = 0;
}

void SerialCLI_Read(SerialCLI *cli, const char *str, size_t length) {
  if (NULL == cli || (NULL == str) || (0 >= length) || SERIAL_CLI_OUTPUT_BUFFER_SIZE < length) {
    return;
  }

  char output[SERIAL_CLI_OUTPUT_BUFFER_SIZE] = {0};

  for (size_t i = 0; i < length; ++i) {
    // Reset the buffer if it is full
    if (SERIAL_CLI_INPUT_BUFFER_SIZE == cli->charCount) {
      resetCLI(cli);
      return;
    }

    switch (str[i]) {
    case CARRIAGE_RETURN:
      cli->state = SERIAL_CLI_PREPROCESSING_INPUT;
      return;
    case BACKSPACE:
      if (cli->charCount > 0) {
        cli->inputBuffer[cli->charCount - 1] = '\0';
        cli->charCount--;
        // Delete the last character from the terminal window
        const char *deleteSequence = "\b \b";
        strncat(output, deleteSequence, strlen(output));
      }
      break;
    default:
      strncat(output, &str[i], strlen(output));
      // Store the character in the input buffer
      cli->inputBuffer[cli->charCount] = str[i];
      cli->charCount++;
      break;
    }
  }

  // Write the input back to the serial interface
  cli->write(output, strlen(output));
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
  if (NULL == cli || NULL == command || NULL == command->commandName || NULL == command->command ||
      cli->commandsCount == (SERIAL_CLI_COMMANDS_MAX + 1)) {
    return false;
  }

  memcpy(&cli->commands[cli->commandsCount], command, sizeof(SerialCLI_CommandEntry));
  ++cli->commandsCount;
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
