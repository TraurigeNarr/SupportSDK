project(PerformanceTests)

set(SOURCES CommandBucketTests.h
    FactoryTests.h
    FSM_Test_HierarchicalvsTemplate.h
    GlobalObjectGetterTests.h
    MessageDispatcherTests.h
    StateMachineTests.h
    stdafx.h
    TempTests.h
    commandBucketTests.cpp
    FactoryTests.cpp
    FSM_Test_HierarchicalvsTemplate.cpp
    GlobalObjcetGetterTests.cpp
    MessageDispatcherTests.cpp
    PerformanceTests.cpp
    StateMachineTests.cpp
    stdafx.cpp
    TempTests.cpp
    )

add_executable(PerformanceTests ${SOURCES})
target_link_libraries(PerformanceTests PRIVATE GameCore)
target_include_directories(PerformanceTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../Sources)

set_target_properties(PerformanceTests PROPERTIES FOLDER "Samples")