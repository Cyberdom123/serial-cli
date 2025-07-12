add_library(
  serial_cli
  STATIC
  serial_cli.c
)

target_include_directories(
  serial_cli
  PUBLIC
  include
  PRIVATE
  include_internal
)
