set(ASSIMP_INSTALL_DIR ${THIRDPARTY_INSTALL_FOLDER}/assimp)
set(ASSIMP_INCLUDE_DIR ${ASSIMP_INSTALL_DIR}/include CACHE PATH INTERNAL)
set(ASSIMP_LIB_DIR ${ASSIMP_INSTALL_DIR}/lib CACHE PATH INTERNAL)

ExternalProject_Add(assimp 
	GIT_REPOSITORY https://github.com/assimp/assimp GIT_TAG "v4.0.1"
	UPDATE_COMMAND ""
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${ASSIMP_INSTALL_DIR} -DASSIMP_BUILD_ASSIMP_TOOLS=OFF)
