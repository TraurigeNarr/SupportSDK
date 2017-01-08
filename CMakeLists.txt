cmake_minimum_required (VERSION 2.6)
#project(SupportSDK)

# The version number.
set (SDK_VERSION_MAJOR 1)
set (SDK_VERSION_MINOR 0)

cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0003 OLD)

#OPTION(WITH_DEBUG_MODE "Build with debug mode" ON)

# path settings
#include (Sources/SupportSDK.cmake)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries)

add_subdirectory(Sources)
add_subdirectory(Samples)