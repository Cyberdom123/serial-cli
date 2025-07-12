#include "serial_cli.h"

#include <atomic>
#include <iostream>
#include <mutex>
#include <stdio.h>
#include <string>
#include <termios.h>
#include <thread>
#include <unistd.h>

namespace {

SerialCLI cli;

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
      // This is a way to exit the program when ctrl+c is pressed
      const int ctrlC = 3;
      if (c == ctrlC) {
        // stop all threads
        stopFlag = true;
      }

      mutex.lock();
      SerialCLI_Read(&cli, &c, 1);
      mutex.unlock();
    }
  }
}

void process(void) {
  while (!stopFlag) {
    mutex.lock();
    SerialCLI_Process(&cli);
    mutex.unlock();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
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
  tcgetattr(STDIN_FILENO, &originalStdin);
  rawStdin = originalStdin;
  cfmakeraw(&rawStdin);
  tcsetattr(STDIN_FILENO, TCSANOW, &rawStdin);

  int ret = 0;
  try {
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
