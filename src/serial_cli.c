#include "serial_cli.h"
#include "serial_cli_commands.h"
#include "serial_cli_internal.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

enum {
  ASCII_CARRIAGE_RETURN = '\r', // ASCII CR character
  ASCII_TAB = '\t',             // ASCII TAB character
  ASCII_DEL = 127,              // ASCII DEL character
};

static void resetCLI(SerialCLI *cli) {
  memset(cli->inputBuffer, 0, SERIAL_CLI_INPUT_BUFFER_SIZE);
  memset(cli->tokens, 0, sizeof(cli->tokens));

  cli->charCount = 0;
  cli->tokenCount = 0;
  cli->isCommandReady = false;

  SerialCLI_WriteString(cli, "\r\n%s ", cli->promptBuffer);
}

static void callCommand(SerialCLI *cli, const char *commandName) {
  SerialCLI_CommandEntry *entry = SerialCLI_GetCommandEntry(cli, commandName);
  if (NULL != entry) {
    char *toWrite = "\r\n";
    SerialCLI_WriteBack(cli, toWrite, strlen(toWrite));

    char *argv[SERIAL_CLI_COMMAND_MAX_ARGS + 1] = {0};
    SerialCLI_GetArgv(cli, argv);
    entry->command(cli, (int)cli->tokenCount, (const char **)argv);
  }
}

static const char *parseInput(SerialCLI *cli) {
  bool isQuotedArgument = false;
  bool isRegularArgument = false;

  size_t argumentIdx = 0;
  size_t argumentLength = 0;
  for (size_t i = 0; i < cli->charCount; ++i) {
    bool isMaxArgLengthReached = (argumentLength > SERIAL_CLI_COMMAND_MAX_ARG_LENGTH);
    bool isMaxArgsReached = (argumentIdx == SERIAL_CLI_COMMAND_MAX_ARGS);
    if (isMaxArgLengthReached || isMaxArgsReached) {
      return NULL;
    }

    // quoted argument
    if ('\"' == cli->inputBuffer[i]) {
      if (isQuotedArgument) {
        ++argumentIdx;
      }
      isQuotedArgument = !isQuotedArgument;
      argumentLength = 0;
      continue;
    }

    if (isQuotedArgument) {
      cli->tokens[argumentIdx][argumentLength] = cli->inputBuffer[i];
      ++argumentLength;
      continue;
    }

    // regular argument
    if (isspace((unsigned char)cli->inputBuffer[i])) {
      if (isRegularArgument) {
        ++argumentIdx;
      }
      isRegularArgument = false;
      argumentLength = 0;
    } else {
      isRegularArgument = true;
      cli->tokens[argumentIdx][argumentLength] = cli->inputBuffer[i];
      ++argumentLength;
    }
  }

  isRegularArgument ? (cli->tokenCount = (argumentIdx + 1)) : (cli->tokenCount = argumentIdx);
  return cli->tokens[0];
}

static void helpCommand(SerialCLI *cli, int argc, const char **argv) {
  (void)argv;

  if (argc > 1) {
    SerialCLI_WriteString(cli, "Usage: help\r\n");
    return;
  }

  SerialCLI_WriteString(cli, "Available commands:\r\n");

  SerialCLI_CommandEntry *current = cli->commands.next;
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
  strncpy(cli->promptBuffer, ">>", SERIAL_CLI_PROMPT_BUFFER_MAX_LENGTH);
  resetCLI(cli);

  SerialCLI_CommandEntry *helpEntry = &cli->commands;
  helpEntry->command = helpCommand;
  helpEntry->commandName = "help";
  helpEntry->commandDescription = "Prints all available commands";
  helpEntry->next = NULL;
  return true;
}

bool SerialCLI_Deinit(SerialCLI *cli) {
  if (NULL == cli) {
    return false;
  }

  resetCLI(cli);
  return true;
}

static void handleDelete(SerialCLI *cli, char *output, size_t *outputLen) {
  if (cli->charCount == 0) {
    return;
  }

  // Remove the last character from the input buffer
  cli->inputBuffer[cli->charCount - 1] = '\0';
  cli->charCount--;

  // Add delete sequence to output buffer
  const char *deleteSequence = "\b \b";
  size_t deleteSeqLen = strlen(deleteSequence);
  bool isOutputSpaceAvailable = ((*outputLen + deleteSeqLen) < SERIAL_CLI_OUTPUT_BUFFER_SIZE);

  if (isOutputSpaceAvailable) {
    size_t outIdx = *outputLen;
    for (size_t i = 0; i < deleteSeqLen; i++) {
      output[outIdx] = deleteSequence[i];
      ++outIdx;
    }
    *outputLen = outIdx;
  }
}

static void handleTabCompletion(SerialCLI *cli, char *output, size_t *outputLen) {
  const char *name = SerialCLI_ResolvePartialCommand(cli, cli->inputBuffer);
  if (NULL == name) {
    return;
  }

  size_t outIdx = *outputLen;
  size_t nameLen = strlen(name);

  bool isOutputSpaceAvailable = ((outIdx + nameLen + strlen(" ")) < SERIAL_CLI_OUTPUT_BUFFER_SIZE);
  bool isInputSpaceAvailable = (nameLen + strlen(" ") < SERIAL_CLI_INPUT_BUFFER_SIZE);
  if (!isOutputSpaceAvailable || !isInputSpaceAvailable) {
    return;
  }

  for (size_t i = cli->charCount; i < nameLen; ++i) {
    cli->inputBuffer[i] = name[i];
    output[outIdx] = name[i];
    ++outIdx;
  }

  output[outIdx] = ' ';
  ++outIdx;
  cli->inputBuffer[nameLen] = ' ';
  cli->charCount = nameLen + strlen(" ");
  *outputLen = outIdx;
}

bool SerialCLI_Read(SerialCLI *cli, const char *str, size_t length) {
  if (NULL == cli || (NULL == str)) {
    return false;
  }

  bool isStateValid = !cli->isCommandReady;
  bool isLengthValid = (SERIAL_CLI_OUTPUT_BUFFER_SIZE > length);
  if (!isStateValid || !isLengthValid) {
    return false;
  }

  char output[SERIAL_CLI_OUTPUT_BUFFER_SIZE + 1] = {0};
  size_t outputIdx = 0;

  for (size_t i = 0; i < length; ++i) {
    if (SERIAL_CLI_INPUT_BUFFER_SIZE == cli->charCount) {
      resetCLI(cli);
      return false;
    }

    if (ASCII_CARRIAGE_RETURN == str[i]) {
      cli->isCommandReady = true;
      return true;
    }

    if (ASCII_DEL == str[i]) {
      handleDelete(cli, output, &outputIdx);
      continue;
    }

    if (ASCII_TAB == str[i]) {
      handleTabCompletion(cli, output, &outputIdx);
      continue;
    }

    output[outputIdx] = str[i];
    ++outputIdx;
    cli->inputBuffer[cli->charCount] = str[i];
    ++cli->charCount;
  }

  // Echo the received characters back
  if (outputIdx > 0) {
    SerialCLI_WriteBack(cli, output, outputIdx);
  }
  return true;
}

bool SerialCLI_WriteString(SerialCLI *cli, const char *format, ...) {
  if (NULL == format || NULL == cli) {
    return false;
  }

  char buffer[SERIAL_CLI_OUTPUT_BUFFER_SIZE];
  va_list arg;
  va_start(arg, format);
  int len = vsnprintf(buffer, sizeof(buffer), format, arg);
  va_end(arg);

  bool isLengthValid = (len < SERIAL_CLI_OUTPUT_BUFFER_SIZE);
  if (len <= 0 || !isLengthValid) {
    return false;
  }

  SerialCLI_WriteBack(cli, buffer, (size_t)len);
  return true;
}

bool SerialCLI_SetPrompt(SerialCLI *cli, const char *prompt) {
  if ((NULL == cli) || (NULL == prompt)) {
    return false;
  }

  strncpy(cli->promptBuffer, prompt, SERIAL_CLI_PROMPT_BUFFER_MAX_LENGTH);
  return true;
}

bool SerialCLI_ResetPrompt(SerialCLI *cli) {
  if (NULL == cli) {
    return false;
  }

  strncpy(cli->promptBuffer, ">>", SERIAL_CLI_PROMPT_BUFFER_MAX_LENGTH);
  return true;
}

bool SerialCLI_RegisterCommand(SerialCLI *cli, SerialCLI_CommandEntry *command) {
  if ((NULL == cli) || (NULL == command) || (NULL == command->commandName) || (NULL == command->command)) {
    return false;
  }

  if (strlen(command->commandName) > SERIAL_CLI_COMMAND_MAX_ARG_LENGTH) {
    return false;
  }

  SerialCLI_CommandEntry *current = &cli->commands;

  while (NULL != current->next) {
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

bool SerialCLI_Process(SerialCLI *cli) {
  if (NULL == cli) {
    return false;
  }

  if (cli->isCommandReady) {
    const char *commandName = parseInput(cli);
    if (NULL != commandName) {
      callCommand(cli, commandName);
    }
    resetCLI(cli);
  }

  return true;
}
