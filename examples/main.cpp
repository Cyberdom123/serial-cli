#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <thread>
#include <iostream>
#include <string>
#include <atomic>

#include "serial_cli.h"

static SerialCLI cli;

// Structures to save the terminal settings for original settings & raw mode
struct termios originalStdin;
struct termios rawStdin;

enum {
  CTRL_C = 3,
};

// Global flag to signal threads to stop
std::atomic<bool> stopFlag(false);

static void consoleWrite(const char *str, size_t len) {
  (void)len;
  write(STDOUT_FILENO, str, len);
}

static void consoleRead() {
  while (!stopFlag) {
    unsigned char c;
    if (read(STDIN_FILENO, &c, 1) > 0) {
      // This is a way to exit the program when ctrl+c is pressed
      if (c == CTRL_C) {
        // stop all threads
        stopFlag = true;
      }

      SerialCLI_Read(&cli, (const char *)&c, 1);
    }
  }
}

static void process() {
  while (!stopFlag) {
    SerialCLI_Process(&cli);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}

void exampleCommand(SerialCLI *cli, int argc, const char **argv) {
  switch (argc) {
    case 1:
      SerialCLI_WriteString(cli, "No arguments provided.\r\n");
      break;
    case 2:
      SerialCLI_WriteString(cli, "One argument provided: %s\r\n", argv[1]);
      break;
    case 3:
      SerialCLI_WriteString(cli, "Two arguments provided: %s, %s\r\n", argv[1], argv[2]);
      break;
    case 4:
      SerialCLI_WriteString(cli, "Three arguments provided: %s, %s, %s\r\n", argv[1], argv[2], argv[3]);
      break;
    case 5:
      SerialCLI_WriteString(cli, "Four arguments provided: %s, %s, %s, %s\r\n", argv[1], argv[2], argv[3], argv[4]);
      break;
  }
}

int main() {
  // Backup the terminal settings, and switch to raw mode
  tcgetattr(STDIN_FILENO, &originalStdin);
  rawStdin = originalStdin;
  cfmakeraw(&rawStdin);
  tcsetattr(STDIN_FILENO, TCSANOW, &rawStdin);

  std::cout << "Press CTRL+c to exit" << std::endl;

  SerialCLI_Init(&cli, consoleWrite);

  SerialCLI_CommandEntry commandEntry;
  commandEntry.command = exampleCommand;
  commandEntry.commandName = "example";
  commandEntry.commandDescription = "Example command.";
  SerialCLI_RegisterCommand(&cli, &commandEntry);

  std::thread consoleReadThread(consoleRead);
  std::thread processThread(process);

  consoleReadThread.join();
  processThread.join();

  // Restore terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &originalStdin);
  return 0;
}
