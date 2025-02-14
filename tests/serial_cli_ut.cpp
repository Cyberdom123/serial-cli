#include <gtest/gtest.h>
#include "serial_cli_fixture.hpp"

#include "serial_cli.h"

TEST(SerialCli, Init) {
  ASSERT_TRUE(true);
  SerialCLI cli;
  auto write = [](const char *str, size_t len) {
    (void)str;
    (void)len;
  };

  ASSERT_FALSE(SerialCLI_Init(nullptr, nullptr));
  ASSERT_FALSE(SerialCLI_Init(&cli, nullptr));
  ASSERT_FALSE(SerialCLI_Init(nullptr, write));
  ASSERT_TRUE(SerialCLI_Init(&cli, write));
}

TEST_F(SerialCLITest, CommandRegistration) {
  SerialCLI_CommandEntry commandEntry;
  commandEntry.command = nullptr;
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;

  ASSERT_FALSE(SerialCLI_RegisterCommand(nullptr, &commandEntry));
  ASSERT_FALSE(SerialCLI_RegisterCommand(&cli, nullptr));
  ASSERT_FALSE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  commandEntry.command = [](SerialCLI *cli, int argc, const char **argv) {
    (void)cli;
    (void)argc;
    (void)argv;
  };

  for (size_t i = 0; i < SERIAL_CLI_COMMANDS_MAX; ++i) {
    ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));
  }

  // Exceed the maximum number of commands
  ASSERT_FALSE(SerialCLI_RegisterCommand(&cli, &commandEntry));
}

static bool isCommandExecuted = false;
static void commandHandler(SerialCLI *cli, int argc, const char **argv) {
  (void)cli;
  // Checking the number of arguments
  EXPECT_EQ(argc, 1);
  ASSERT_STREQ(argv[0], "test");
  isCommandExecuted = true;
}

TEST_F(SerialCLITest, ProcessCommand) {
  // Initialize the command entry
  SerialCLI_CommandEntry commandEntry;
  commandEntry.command = commandHandler;
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;
  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  struct TestInput {
    std::string input;
    bool expectedResult;
  };

  TestInput testInputs[] = {
    {"tset\r", false},
    {"test\r", true},
    {" test\r", true},
    {"  test\r", true},
    {" test\r", true},
    {"test\r\n", true},
    {"test\n", false},
    {"test", false},
    {"\rtest", false},
    {"\rtest\r", false}, // Second \r should be ignored 
    {"\ntest", false},
    {"\ntest\r", true},
    {"\r\ntest", false},
    {"\r\ntest\r", false},
    {"\r\n", false},
    {"\r", false},
    {"\n", false},
    {"test\r arg", true},
    {"test\r test\r", true},
    {"test\r\n arg", true},
    {"test\r\n test\r\n", true},
    {"test\n arg", false},
  };

  for (const auto &testInput : testInputs) {
    isCommandExecuted = false;
    WriteString(testInput.input.c_str());
    Process();
    EXPECT_EQ(isCommandExecuted, testInput.expectedResult) << "Input: " << testInput.input;

    // Reset terminal input
    std::string deleteKey(1, '\177');
    for (size_t i = 0; i < testInput.input.size(); ++i) {
      WriteString(deleteKey);
    }
  }
}

static void NonAlphanumericCommandHandler(SerialCLI *cli, int argc, const char **argv) {
  (void)cli;
  (void)argv;
  // Checking the number of arguments
  EXPECT_EQ(argc, 1);
  isCommandExecuted = true;
}

TEST_F(SerialCLITest, NonAlphanumericCommand) {
  // Initialize the command entry properly
  SerialCLI_CommandEntry commandEntry;
  commandEntry.command = NonAlphanumericCommandHandler;
  commandEntry.commandName = "test!";
  commandEntry.commandDescription = nullptr;

  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));


  isCommandExecuted = false;
  WriteString("test!\r");
  Process();
  EXPECT_EQ(isCommandExecuted, true);

  SerialCLI_CommandEntry commandEntry2;
  commandEntry2.command = NonAlphanumericCommandHandler;
  commandEntry2.commandName = "test@";
  commandEntry2.commandDescription = nullptr;

  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry2));

  isCommandExecuted = false;
  WriteString("test@\r");
  Process();
  EXPECT_EQ(isCommandExecuted, true);
}

static void handleMultipleCommandArgs(SerialCLI *cli, int argc, const char **argv) {
  (void)cli;

  // Checking the number of arguments
  EXPECT_LE(argc, 11);
  ASSERT_STREQ(argv[0], "test");
  isCommandExecuted = true;
}

TEST_F(SerialCLITest, MultipleArguments) {
  // Initialize the command entry properly
  SerialCLI_CommandEntry commandEntry;
  commandEntry.command = handleMultipleCommandArgs;
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;

  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  // Check for all possible number of arguments
  for (int i = 0; i < SERIAL_CLI_COMMAND_MAX_ARGS; ++i) {
    std::string testInput = "test";
    for (int j = 0; j < i; j++) {
      testInput += " arg";
    }
    testInput += "\r";

    isCommandExecuted = false;
    WriteString(testInput.c_str());
    Process();
    EXPECT_EQ(isCommandExecuted, true) << "Input: " << testInput;
  }

  // Exceed the maximum number of arguments
  for (int i = SERIAL_CLI_COMMAND_MAX_ARGS; i < SERIAL_CLI_COMMAND_MAX_ARGS + 10; ++i) {
    std::string testInput = "test";
    for (int j = 0; j < i; j++) {
      testInput += " a";
    }
    testInput += "\r";

    isCommandExecuted = false;
    WriteString(testInput.c_str());
    Process();
    EXPECT_EQ(isCommandExecuted, false) << "Input: " << testInput;
  }
}

TEST_F(SerialCLITest, ExceedingCommandBuffer) {
  // Initialize the command entry properly
  SerialCLI_CommandEntry commandEntry;
  commandEntry.command = handleMultipleCommandArgs;
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;

  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  // Exceed the maximum command buffer size
  std::string testInput = "test";
  for (int i = 0; i < SERIAL_CLI_INPUT_BUFFER_SIZE; i++) {
    testInput += " a";
  }
  testInput += "\r";

  isCommandExecuted = false;
  WriteString(testInput.c_str());
  Process();
  EXPECT_EQ(isCommandExecuted, false) << "Input: " << testInput;
}

TEST_F(SerialCLITest, ExceedingArgumentBuffer) {
  // Initialize the command entry properly
  SerialCLI_CommandEntry commandEntry;
  commandEntry.command = handleMultipleCommandArgs;
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;

  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  // Maximum argument buffer size
  std::string testInput = "test ";
  for (int i = 0; i < SERIAL_CLI_COMMAND_MAX_ARG_LENGTH; i++) {
    testInput += "a";
  }
  testInput += "\r";
  isCommandExecuted = false;
  WriteString(testInput.c_str());
  Process();
  EXPECT_EQ(isCommandExecuted, true) << "Input: " << testInput;

  // Exceed the maximum argument buffer size
  testInput = "test ";
  for (int i = 0; i < SERIAL_CLI_COMMAND_MAX_ARG_LENGTH + 10; i++) {
    testInput += "a";
  }
  testInput += "\r";
  isCommandExecuted = false;
  WriteString(testInput.c_str());
  Process();
  EXPECT_EQ(isCommandExecuted, false) << "Input: " << testInput;
}