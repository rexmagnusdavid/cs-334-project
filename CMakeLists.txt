cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(main)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(EXECUTABLE_OUTPUT_PATH  ${CMAKE_BINARY_DIR})
# execute subdirectory cmakelists
add_subdirectory(${PROJECT_SOURCE_DIR}/external)
# src
aux_source_directory(${PROJECT_SOURCE_DIR}/src SOURCE_DIR)
# exe
add_executable(${PROJECT_NAME} ${SOURCE_DIR})
# lib
target_link_libraries(${PROJECT_NAME} deps_shared_lib)
