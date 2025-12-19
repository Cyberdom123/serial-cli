#include <gtest/gtest.h>

#include "serial_cli.h"

class SerialCLITest : public ::testing::Test {
public:
  SerialCLI cli;

  void process() {
    // Processes enough times to handle a full command and any extra input
    constexpr size_t maxIterations = SERIAL_CLI_COMMAND_MAX_ARGS + 2;
    for (size_t i = 0; i < maxIterations; ++i) {
      SerialCLI_Process(&cli);
    }
  }

  void writeString(std::string_view str) {
    for (char ch : str) {
      SerialCLI_Read(&cli, &ch, 1);
    }
  }

protected:
  void SetUp() override {
    SerialCLI_Init(&cli, [](const char *, size_t) {});
  }

  void TearDown() override { SerialCLI_Deinit(&cli); }
};