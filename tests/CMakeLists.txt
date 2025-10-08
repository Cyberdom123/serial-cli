FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
  unit_tests
  serial_cli_ut.cpp
)

target_include_directories(
  unit_tests
  PRIVATE
  include
)

target_link_libraries(
  unit_tests
  PRIVATE
  serial_cli
  GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(unit_tests)