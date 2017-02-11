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

function(SetVS_WorkingDir vcx_file_name)
if (${CMAKE_GENERATOR} MATCHES "Visual Studio 14.+" OR ${CMAKE_GENERATOR} MATCHES "Visual Studio 15.+")
#due to http://stackoverflow.com/questions/23950887/does-cmake-offer-a-method-to-set-the-working-directory-for-a-given-build-system
file( WRITE ${vcx_file_name}
     "<?xml version=\"1.0\" encoding=\"utf-8\"?>     \
    <Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
    <PropertyGroup>
        <LocalDebuggerWorkingDirectory>$(TargetDir)</LocalDebuggerWorkingDirectory>
        <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
    </PropertyGroup>
    </Project>")
endif()
endfunction()

add_subdirectory(Sources)
add_subdirectory(Samples)