
file(GLOB all_src_files "*.cpp" "*.hpp" "../include/GE/*.hpp")

add_library(${ENGINE_NAME} STATIC ${all_src_files})

source_group(Camera FILES
../include/GE/Camera.hpp
../include/GE/OrthoCamera.hpp
../include/GE/PerspectiveCamera.hpp
../include/GE/RawCamera.hpp

Camera.cpp
OrthoCamera.cpp
PerspectiveCamera.cpp
RawCamera.cpp
)

source_group(GraphicalObject FILES
../include/GE/AttributeCategory.hpp
../include/GE/GraphicalObject.hpp
../include/GE/Mesh.hpp
MeshManager.hpp
StandardGraphicalObject.hpp

AttributeCategory.cpp
Mesh.cpp
MeshManager.cpp
StandardGraphicalObject.cpp
)

source_group(Materials FILES
../include/GE/Material.hpp
../include/GE/SimpleColoredMaterial.hpp
MaterialManager.hpp
StandardMaterial.hpp

Material.cpp
MaterialManager.cpp
SimpleColoredMaterial.cpp
StandardMaterial.cpp
)

include_directories("../include")
include_directories(${SFML_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GL_INCLUDE_DIRS})

target_link_libraries(${ENGINE_NAME} ${SFML_LIBRARIES})
target_link_libraries(${ENGINE_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${ENGINE_NAME} ${OPENGL_LIBRARIES})
