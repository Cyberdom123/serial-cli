project(serial_cli_examples) 
enable_language(C CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../source ${CMAKE_CURRENT_BINARY_DIR}/source)

add_executable(serial_cli_examples main.cpp)

target_link_libraries(serial_cli_examples serial_cli)
