cmake_minimum_required(VERSION 3.22)
project(serial_cli)

option(UNIT_TESTING "Enable unit testing" ${PROJECT_IS_TOP_LEVEL})
option(EXAMPLES "Build examples" ${PROJECT_IS_TOP_LEVEL})

include(requirements.cmake)

add_subdirectory(src)

if(UNIT_TESTING)
  add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

if(EXAMPLES)
  add_subdirectory(examples EXCLUDE_FROM_ALL)
endif()
