add_library(
  serial_cli
  serial_cli.c
)

target_include_directories(
  serial_cli
  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/Include
)
