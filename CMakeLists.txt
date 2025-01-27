cmake_minimum_required(VERSION 3.22)
project(serial_cli)

enable_language(C CXX)

set(UNIT_TESTING OFF CACHE BOOL "Enable unit testing")

include(requirements.cmake)
FetchContent_MakeAvailable(googletest)

if (UNIT_TESTING)
  enable_testing()
  add_subdirectory(Tests)
elseif()
  add_subdirectory(Source)
endif()
