#include "serial_cli.h"

#include <assert.h>

bool SERIAL_CLI_Init(SERIAL_CLI *cli) {
  assert(NULL != cli);

  if (NULL == cli) {
    return false;
  }
  return true;
}

void SERIAL_CLI_Deinit(SERIAL_CLI *cli) {
  assert(NULL != cli);

  if (NULL == cli) {
    return;
  }
}

bool SERIAL_CLI_RegisterCommand(SERIAL_CLI *cli, SERIAL_CLI_CommandEntry *command) {
  assert(NULL != cli);
  assert(NULL != command);

  if (NULL == cli || NULL == command) {
    return false;
  }
  return true;
}

void SERIAL_CLI_Process(SERIAL_CLI *cli) {
  assert(NULL != cli);
  if (NULL == cli) {
    return;
  }
}