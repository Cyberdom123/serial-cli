#include <gtest/gtest.h>

#include "serial_cli.h"

TEST(SerialCli, Init) {
  SERIAL_CLI cli;

  ASSERT_DEATH(SERIAL_CLI_Init(nullptr), "");
  ASSERT_TRUE(SERIAL_CLI_Init(&cli));
  ASSERT_NO_THROW(SERIAL_CLI_Init(&cli));
}