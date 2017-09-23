project(RenderTesting)

file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

add_executable(RenderTesting WIN32 ${HEADER_FILES} ${SOURCE_FILES})

target_link_libraries(RenderTesting PRIVATE GameCore)
target_include_directories(RenderTesting PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../../../Sources
	${CMAKE_CURRENT_SOURCE_DIR}/../../../Sources/GameCore)

set_target_properties(RenderTesting PROPERTIES FOLDER "Samples")