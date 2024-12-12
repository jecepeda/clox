cmake_minimum_required(VERSION 3.10)

# Set the project name
project(clox)

# Specify the C standard
set(CMAKE_C_STANDARD 99)

# Include directories
include_directories(include)

# Add the source files
file(GLOB SOURCES "src/*.c")

# standard flags
set(CMAKE_C_FLAGS, "${CMAKE_C_FLAGS} -Wall -Wextra -D_GNU_SOURCE")

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Configuring Debug build")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Configuring Release build")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Os")
endif()

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../bin)

# Add the executable
add_executable(clox ${SOURCES})