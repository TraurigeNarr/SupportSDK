file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

file(GLOB DTREES_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/DecisionTrees/*.h")
file(GLOB DTREES_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/DecisionTrees/*.cpp")

add_library(DecisionMaking SHARED ${HEADER_FILES} ${SOURCE_FILES} ${DTREES_HEADER_FILES} ${DTREES_SOURCE_FILES})
target_include_directories(DecisionMaking PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

if(WITH_TESTS)
	file(GLOB TEST_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/DecisionMaking.Tests/*.cpp")
	add_executable(DecisionMaking.Tests ${TEST_SOURCE_FILES})
	target_include_directories(DecisionMaking.Tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${GTEST_INCLUDE_DIR})
	target_link_libraries(DecisionMaking.Tests DecisionMaking ${GTEST_LIB_DIR}/gtest.lib ${GTEST_LIB_DIR}/gmock.lib)
	add_dependencies(DecisionMaking.Tests gtest)
	add_test(NAME DecisionMaking.Tests COMMAND DecisionMaking.Tests)
	set_target_properties(DecisionMaking.Tests PROPERTIES FOLDER "Tests")
endif()
