#include "serial_cli.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>

using namespace std::chrono_literals;

namespace {

SerialCLI cli;
SerialCLI_CommandEntry commandEntry;

// Global flag to signal threads to stop
std::atomic<bool> stopFlag(false);
std::mutex mutex;

// Structures to save the terminal settings for original settings & raw mode
struct termios originalStdin;
struct termios rawStdin;

void consoleWrite(const char *str, size_t len) { write(STDOUT_FILENO, str, len); }

void consoleRead(void) {
  while (!stopFlag) {
    char c;
    if (read(STDIN_FILENO, &c, 1) > 0) {
      // Exit the program when Ctrl+C is pressed (ASCII ETX character)
      constexpr int CTRL_C = 3;
      if (c == CTRL_C) {
        std::cout << "\nReceived Ctrl+C, shutting down..." << std::endl;
        stopFlag = true;
      }

      if (std::unique_lock<std::mutex> lock(mutex, std::try_to_lock); lock.owns_lock()) {
        SerialCLI_Read(&cli, &c, 1);
      }
    }
  }
}

void process(void) {
  while (!stopFlag) {
    if (std::unique_lock<std::mutex> lock(mutex, std::try_to_lock); lock.owns_lock()) {
      SerialCLI_Process(&cli);
    }
    std::this_thread::sleep_for(10ms);
  }
}

void exampleCommand(SerialCLI *cli, int argc, const char **argv) {
  if (argc <= 1) {
    SerialCLI_WriteString(cli, "No arguments provided.\r\n");
    return;
  }
  SerialCLI_WriteString(cli, "%d argument provided:", argc - 1);
  for (int i = 1; i < argc; ++i) {
    SerialCLI_WriteString(cli, " %s%s", argv[i], (i < argc - 1) ? "," : "");
  }
  SerialCLI_WriteString(cli, "\r\n");
}

} // namespace

int main() {
  // Backup the terminal settings, and switch to raw mode
  if (tcgetattr(STDIN_FILENO, &originalStdin) != 0) {
    std::cerr << "Failed to get terminal attributes" << std::endl;
    return 1;
  }

  rawStdin = originalStdin;
  cfmakeraw(&rawStdin);

  if (tcsetattr(STDIN_FILENO, TCSANOW, &rawStdin) != 0) {
    std::cerr << "Failed to set terminal to raw mode" << std::endl;
    return 1;
  }

  int ret = 0;
  try {
    std::cout << "Press CTRL+c to exit" << std::endl;

    SerialCLI_Init(&cli, consoleWrite);

    commandEntry.command = exampleCommand;
    commandEntry.commandName = "example";
    commandEntry.commandDescription = "Example command.";
    SerialCLI_RegisterCommand(&cli, &commandEntry);

    std::thread consoleReadThread(consoleRead);
    std::thread processThread(process);

    consoleReadThread.join();
    processThread.join();
  } catch (const std::exception &ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
    ret = 1;
  } catch (...) {
    std::cerr << "Unknown exception occurred." << std::endl;
    ret = 1;
  }

  // Restore terminal settings
  tcsetattr(STDIN_FILENO, TCSANOW, &originalStdin);
  return ret;
}
