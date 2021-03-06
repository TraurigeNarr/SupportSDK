file(GLOB HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.h")
file(GLOB SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

file(GLOB FACTORY_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Factory/*.h")
file(GLOB FACTORY_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/Factory/*.cpp")

file(GLOB MESSAGE_DISPATCHER_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/MessageDispatcher/*.h")
file(GLOB MESSAGE_DISPATCHER_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/MessageDispatcher/*.cpp")

file(GLOB STATE_MACHINE_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/StateMachine/*.h")
file(GLOB STATE_MACHINE_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/StateMachine/*.cpp")

add_library(Patterns SHARED 
    ${HEADER_FILES} ${SOURCE_FILES} 
    ${FACTORY_HEADER_FILES} ${FACTORY_SOURCE_FILES}
    ${MESSAGE_DISPATCHER_HEADER_FILES} ${MESSAGE_DISPATCHER_SOURCE_FILES}
    ${STATE_MACHINE_HEADER_FILES} ${STATE_MACHINE_SOURCE_FILES})
target_include_directories(Patterns PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/..)

set_source_files_properties(${FACTORY_HEADER_FILES} ${FACTORY_SOURCE_FILES} PROPERTIES FOLDER Factory)