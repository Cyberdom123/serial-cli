#include <gtest/gtest.h>

#include "serial_cli.h"

static std::string buffer;

class SerialCLITest : public ::testing::Test {
public:
    SerialCLI cli;

    void Process() {
        for (size_t i = 0; i < (SERIAL_CLI_COMMAND_MAX_ARGS + 2); i++) {
            SerialCLI_Process(&cli);
        }
    }

    void WriteString(std::string str) {
        SerialCLI_Read(&cli, str.c_str(), str.size());
    }

private:
    static void Write(const char *str, size_t len) {
        buffer.append(str, len);
    }

protected:
    virtual void SetUp() {
        SerialCLI_Init(&cli, Write);
    }

    virtual void TearDown() {
        SerialCLI_Deinit(&cli);
    }
};