
file(GLOB all_src_files "*.cpp" "../include/GE/*.hpp")

add_library(${ENGINE_NAME} STATIC ${all_src_files})

source_group(Camera FILES
Camera.cpp
Camera.hpp
OrthoCamera.cpp
OrthoCamera.hpp
PerspectiveCamera.cpp
PerspectiveCamera.hpp
RawCamera.cpp
RawCamera.hpp
)

include_directories("../include")
include_directories(${SFML_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GL_INCLUDE_DIRS})

target_link_libraries(${ENGINE_NAME} ${SFML_LIBRARIES})
target_link_libraries(${ENGINE_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${ENGINE_NAME} ${OPENGL_LIBRARIES})
