#include "serial_cli_fixture.hpp"
#include <gtest/gtest.h>

#include "serial_cli.h"

TEST(SerialCli, Init) {
  SerialCLI cli;
  auto write = [](const char *str, size_t len) {};

  ASSERT_DEATH(ASSERT_FALSE(SerialCLI_Init(nullptr, nullptr)), "");
  ASSERT_DEATH(ASSERT_FALSE(SerialCLI_Init(&cli, nullptr)), "");
  ASSERT_DEATH(ASSERT_FALSE(SerialCLI_Init(nullptr, write)), "");
  ASSERT_TRUE(SerialCLI_Init(&cli, write));
}

TEST_F(SerialCLITest, CommandRegistration) {
  SerialCLI_CommandEntry command = {
      .command = nullptr,
      .command_name = "test",
      .command_description = nullptr,
  };

  ASSERT_DEATH(ASSERT_FALSE(SerialCLI_RegisterCommand(nullptr, &command)), "");
  ASSERT_DEATH(ASSERT_FALSE(SerialCLI_RegisterCommand(&cli, nullptr)), "");
  ASSERT_DEATH(ASSERT_FALSE(SerialCLI_RegisterCommand(&cli, &command)), "");

  command.command = [](int argc, const char **argv) {};

  ASSERT_NO_THROW(ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &command)));
}

static bool isCommandExecuted = false;
static void commandHandler(int argc, const char **argv) {
  ASSERT_EQ(argc, 1);
  ASSERT_STREQ(argv[0], "test");
  isCommandExecuted = true;
}

TEST_F(SerialCLITest, ProcessCommand) {
  // Initialize the command entry properly
  SerialCLI_CommandEntry command;
  command.command = commandHandler;
  command.command_name = "test";
  command.command_description = nullptr;

  // Register the command
  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &command)) << "Failed to register command";

  // Ensure that isCommandExecuted is false
  isCommandExecuted = false;

  // Simulate input
  WriteString("bad-command\r");
  SerialCLI_Process(&cli);

  // Ensure the command was not executed
  ASSERT_FALSE(isCommandExecuted) << "Wrong command was executed";

  // Simulate input
  WriteString("test\r");
  SerialCLI_Process(&cli);

  // Ensure the command was executed
  ASSERT_TRUE(isCommandExecuted) << "Command was not executed";
}