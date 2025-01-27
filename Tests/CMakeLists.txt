cmake_minimum_required(VERSION 3.14)

project(unit_tests)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../Source ${CMAKE_CURRENT_BINARY_DIR}/Source)

add_executable(
  unit_tests
  serial_cli_ut.cpp
)

target_link_libraries(
  unit_tests
  GTest::gtest_main
  serial_cli
)

include(GoogleTest)
gtest_discover_tests(unit_tests)