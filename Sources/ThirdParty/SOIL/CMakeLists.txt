set(SOIL_INSTALL_DIR ${THIRDPARTY_INSTALL_FOLDER}/SOIL)
set(SOIL_INCLUDE_DIR ${SOIL_INSTALL_DIR}/include CACHE PATH INTERNAL)
set(SOIL_LIB_DIR ${SOIL_INSTALL_DIR}/lib CACHE PATH INTERNAL)

ExternalProject_Add(SOIL 
    URL http://www.lonesock.net/files/soil.zip 
	UPDATE_COMMAND ""
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/SOIL
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/update/CMakeLists.txt ${CMAKE_CURRENT_BINARY_DIR}/SOIL/src/SOIL/CMakeLists.txt
    CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${SOIL_INSTALL_DIR}
)
