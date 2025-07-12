#include <gtest/gtest.h>

#include "serial_cli.h"
#include "serial_cli_fixture.hpp"

TEST(SerialCli, Init) {
  ASSERT_TRUE(true);
  SerialCLI cli;
  auto write = [](const char *, size_t) {};

  ASSERT_FALSE(SerialCLI_Init(nullptr, nullptr));
  ASSERT_FALSE(SerialCLI_Init(&cli, nullptr));
  ASSERT_FALSE(SerialCLI_Init(nullptr, write));
  ASSERT_TRUE(SerialCLI_Init(&cli, write));
}

TEST_F(SerialCLITest, CommandRegistration) {
  SerialCLI_CommandEntry commandEntry{};
  commandEntry.command = nullptr;
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;

  ASSERT_FALSE(SerialCLI_RegisterCommand(nullptr, &commandEntry));
  ASSERT_FALSE(SerialCLI_RegisterCommand(&cli, nullptr));
  ASSERT_FALSE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  commandEntry.command = [](SerialCLI *, int, const char **) {};

  for (auto i = 0; i < SERIAL_CLI_COMMANDS_MAX; ++i) {
    ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));
  }
  // Exceed the maximum number of commands
  ASSERT_FALSE(SerialCLI_RegisterCommand(&cli, &commandEntry));
}

TEST_F(SerialCLITest, processCommand) {
  static bool isCommandExecuted = false;

  SerialCLI_CommandEntry commandEntry{};
  commandEntry.command = [](SerialCLI *, int argc, const char **argv) {
    EXPECT_EQ(argc, 1);
    ASSERT_STREQ(argv[0], "test");

    isCommandExecuted = true;
  };
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;

  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  struct TestInput {
    std::string input;
    bool expectedResult;
  };

  TestInput testInputs[] = {
      {"tset\r", false},       {"test\r", true},       {" test\r", true},
      {"  test\r", true},      {" test\r", true},      {"test\r\n", true},
      {"test\n", false},       {"test", false},        {"\rtest", false},
      {"\rtest\r", false},     {"\ntest", false},      {"\ntest\r", true},
      {"\r\ntest", false},     {"\r\ntest\r", false},  {"\r\n", false},
      {"\r", false},           {"\n", false},          {"test\r arg", true},
      {"test\r test\r", true}, {"test\r\n arg", true}, {"test\r\n test\r\n", true},
      {"test\n arg", false},
  };

  for (const auto &testInput : testInputs) {
    isCommandExecuted = false;
    writeString(testInput.input.c_str());
    process();

    EXPECT_EQ(isCommandExecuted, testInput.expectedResult) << "Input: " << testInput.input;

    // Simulate deleting the input by writing backspace characters
    for (size_t i = 0; i < testInput.input.size(); ++i) {
      writeString("\177");
    }
  }
}

TEST_F(SerialCLITest, CommandNamesWithSpecialCharacters) {
  static bool isCommandExecuted = false;

  auto handler = [](SerialCLI *, int argc, const char **) {
    EXPECT_EQ(argc, 1);
    isCommandExecuted = true;
  };

  SerialCLI_CommandEntry entry;
  entry.command = handler;
  entry.commandDescription = nullptr;

  entry.commandName = "test!";
  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &entry));
  writeString("test!\r");
  process();
  EXPECT_TRUE(isCommandExecuted);

  entry.commandName = "test@";
  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &entry));
  isCommandExecuted = false;
  writeString("test@\r");
  process();
  EXPECT_TRUE(isCommandExecuted);
}

TEST_F(SerialCLITest, MultipleArguments) {
  static bool isCommandExecuted = false;

  SerialCLI_CommandEntry commandEntry{};
  commandEntry.command = [](SerialCLI *, int argc, const char **argv) {
    EXPECT_LE(argc, SERIAL_CLI_COMMAND_MAX_ARGS);
    ASSERT_STREQ(argv[0], "test");
    isCommandExecuted = true;
  };
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;
  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  // Test up to the maximum number of arguments
  for (int i = 0; i < SERIAL_CLI_COMMAND_MAX_ARGS; ++i) {
    std::string testInput = "test";
    for (int j = 0; j < i; ++j) {
      testInput += " arg";
    }
    testInput += "\r";

    isCommandExecuted = false;
    writeString(testInput.c_str());
    process();
    EXPECT_TRUE(isCommandExecuted) << "Input: " << testInput;
  }

  // Exceed the maximum number of arguments
  for (int i = SERIAL_CLI_COMMAND_MAX_ARGS; i < SERIAL_CLI_COMMAND_MAX_ARGS + 10; ++i) {
    std::string testInput = "test";
    for (int j = 0; j < i; ++j) {
      testInput += " arg";
    }
    testInput += "\r";

    isCommandExecuted = false;
    writeString(testInput.c_str());
    process();
    EXPECT_FALSE(isCommandExecuted) << "Input: " << testInput;
  }
}

TEST_F(SerialCLITest, ExceedingCommandBuffer) {
  SerialCLI_CommandEntry commandEntry{};
  commandEntry.command = [](SerialCLI *, int, const char **) { FAIL() << "Command should not be executed"; };
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;
  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  // Exceed the maximum command buffer size
  std::string testInput = "test";
  for (auto i = 0; i < SERIAL_CLI_INPUT_BUFFER_SIZE; ++i) {
    testInput += " arg";
  }
  testInput += "\r";

  writeString(testInput.c_str());
  process();
}

TEST_F(SerialCLITest, ExceedingArgumentBuffer) {
  static bool isCommandExecuted = false;

  SerialCLI_CommandEntry commandEntry{};
  commandEntry.command = [](SerialCLI *, int argc, const char **argv) {
    EXPECT_LE(argc, SERIAL_CLI_COMMAND_MAX_ARGS);
    ASSERT_STREQ(argv[0], "test");
    isCommandExecuted = true;
  };
  commandEntry.commandName = "test";
  commandEntry.commandDescription = nullptr;

  ASSERT_TRUE(SerialCLI_RegisterCommand(&cli, &commandEntry));

  // Test with maximum allowed argument length
  std::string testInput = "test ";
  testInput.append(SERIAL_CLI_COMMAND_MAX_ARG_LENGTH, 'a');
  testInput += "\r";

  writeString(testInput.c_str());
  process();
  EXPECT_TRUE(isCommandExecuted) << "Input: " << testInput;

  // Test exceeding the maximum argument length
  testInput = "test ";
  testInput.append(SERIAL_CLI_COMMAND_MAX_ARG_LENGTH + 10, 'a');
  testInput += "\r";

  isCommandExecuted = false;
  writeString(testInput.c_str());
  process();
  EXPECT_FALSE(isCommandExecuted) << "Input: " << testInput;
}
