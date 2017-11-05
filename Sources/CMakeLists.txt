#root
cmake_minimum_required (VERSION 2.6)

project(SupportSDK)

set(PROJECT SupportSDK)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# The version number.
set(SDK_VERSION_MAJOR 1)
set(SDK_VERSION_MINOR 0)

#cmake_policy(SET CMP0011 NEW)
#cmake_policy(SET CMP0003 OLD)

option(WITH_TESTS "Build tests" ON)

if(WITH_TESTS)
	enable_testing()
endif()

# setup (could be moved to .cmake file)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Binaries)
add_definitions(-DUNICODE)

# Add third party modules
add_subdirectory(ThirdParty)

# Add components
add_subdirectory(DecisionMaking)
add_subdirectory(ExecutionManagement)
add_subdirectory(GameCore)
add_subdirectory(Graphs)
add_subdirectory(Math)
add_subdirectory(Network)
add_subdirectory(Patterns)
add_subdirectory(Terrain)
add_subdirectory(Utilities)
add_subdirectory(WorldInterfacing)
