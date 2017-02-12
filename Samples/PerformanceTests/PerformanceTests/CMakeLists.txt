project(PerformanceTests)

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(PerformanceTests ${HEADER_FILES} ${SOURCE_FILES})
target_link_libraries(PerformanceTests PRIVATE GameCore)
target_include_directories(PerformanceTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../Sources)

set_target_properties(PerformanceTests PROPERTIES FOLDER "Samples")