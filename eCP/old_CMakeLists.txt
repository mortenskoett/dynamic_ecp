cmake_minimum_required(VERSION 3.18)

project(
    ECP
    VERSION 1.0
    DESCRIPTION "eCP algorithm C++ implementation"
    LANGUAGES CXX)

set (CMAKE_CXX_STANDARD 14)
set( CMAKE_VERBOSE_MAKEFILE on )

# Must be in main file
enable_testing()

# The compiled library code, executable code, external libs and tests
add_subdirectory(src)
add_subdirectory(app)
add_subdirectory(swig)
add_subdirectory(extern)
add_subdirectory(tests)

message("Input C++ compiler flags: ${CMAKE_CXX_FLAGS}")

# Setting build flags
list(APPEND cflags "-O3" "-fPIC" "-Wall" "-fno-rtti")

if(NOT WIN32)
  list(APPEND cflags "-Wextra" "-Wpedantic")
endif()
