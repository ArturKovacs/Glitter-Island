
file(GLOB all_src_files "*.cpp" "*.hpp")

add_executable(${EXECUTABLE_NAME} ${all_src_files})

source_group(GUIContext FILES
BaseDemoContext.cpp
BaseDemoContext.hpp
EditorContext.cpp
EditorContext.hpp
GUIContext.cpp
GUIContext.hpp
ModelSelectionContext.cpp
ModelSelectionContext.hpp
ContextManager.cpp
ContextManager.hpp)

include_directories("../Glitter-Engine/include")
target_link_libraries(${EXECUTABLE_NAME} ${ENGINE_NAME})

include_directories(${SFML_INCLUDE_DIR})
include_directories(${GLEW_INCLUDE_DIRS})
include_directories(${GL_INCLUDE_DIRS})

target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})
target_link_libraries(${EXECUTABLE_NAME} ${GLEW_LIBRARIES})
target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
