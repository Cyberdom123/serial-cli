cmake_minimum_required(VERSION 3.22)
project(serial_cli)

enable_language(C CXX)

option(UNIT_TESTING "Enable unit testing" OFF)
option(EXAMPLES "Build examples" OFF)

include(requirements.cmake)

if (UNIT_TESTING)
  FetchContent_MakeAvailable(googletest)
  enable_testing()
  add_subdirectory(tests EXCLUDE_FROM_ALL)
elseif(EXAMPLES)
  add_subdirectory(examples)
else()
  add_subdirectory(source)
endif()
