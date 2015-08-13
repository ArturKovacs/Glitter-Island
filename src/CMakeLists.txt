
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

source_group(GraphicalObject FILES
AttributeCategory.cpp
AttributeCategory.hpp
GraphicalObject.cpp
GraphicalObject.hpp
Material.cpp
Material.hpp
Mesh.cpp
Mesh.hpp
StandardMaterial.cpp
StandardMaterial.hpp
DepthOnlyMaterial.cpp
DepthOnlyMaterial.hpp)

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

add_library(sfml-graphics UNKNOWN IMPORTED)
add_library(sfml-window UNKNOWN IMPORTED)
add_library(sfml-system UNKNOWN IMPORTED)
add_library(sfml-graphics-d UNKNOWN IMPORTED)
add_library(sfml-window-d UNKNOWN IMPORTED)
add_library(sfml-system-d UNKNOWN IMPORTED)
add_library(GLEW UNKNOWN IMPORTED)

if (${USE_PROVIDED_OGLPLUS})
	message("Using provided oglplus.")
	include_directories(${CMAKE_SOURCE_DIR}/extlibs/oglplus/include)
endif()

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
	# using Clang or GCC
	set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "" FORCE)
	
	# Detect and add SFML
	set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
	find_package (SFML 2.1 COMPONENTS system window graphics)
	if (SFML_FOUND)
		include_directories(${SFML_INCLUDE_DIR})
		target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})
	else ()
		message("Could not find SFML. Setting link libraries anyway, however this project will not build if SFML libraries can not be found by the compiler. (It's recommended to use Clang)")
		set(SFML_LIBRARY_LINKER_FLAGS "-lsfml-graphics -lsfml-window -lsfml-system")
		set_property(TARGET ${EXECUTABLE_NAME} APPEND_STRING PROPERTY LINK_FLAGS "${SFML_LIBRARY_LINKER_FLAGS}")
	endif ()
	
	find_package(GLEW)
	if (GLEW_FOUND)
		include_directories(${GLEW_INCLUDE_DIRS})
		target_link_libraries(${EXECUTABLE_NAME} ${GLEW_LIBRARIES})
	else ()
		message("Could not find GLEW. Setting link libraries anyway, however this project will not build if GLEW can not be found by the compiler. (It's recommended to use Clang)")
		set(GLEW_LIBRARY_LINKER_FLAGS "-lGLEW")
		set_property(TARGET ${EXECUTABLE_NAME} APPEND_STRING PROPERTY LINK_FLAGS "${GLEW_LIBRARY_LINKER_FLAGS}")
	endif ()
	
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	# message("No support for MSVC at this time")
	if(${USE_PROVIDED_OGLPLUS} AND (${MSVC_VERSION} LESS 1800))
		message(FATAL_ERROR "Provided oglplus headers are set up for VS 2013. Please use Visual Studio of at least version 12 (2013) or install oglplus from source.")
	endif()
	
	set(SFML_LIB_DIR ${CMAKE_SOURCE_DIR}/extlibs/SFML/lib)
	set_property(TARGET sfml-graphics PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-graphics.lib)
	set_property(TARGET sfml-window PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-window.lib)
	set_property(TARGET sfml-system PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-system.lib)
	set_property(TARGET sfml-graphics-d PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-graphics-d.lib)
	set_property(TARGET sfml-window-d PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-window-d.lib)
	set_property(TARGET sfml-system-d PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-system-d.lib)
	target_link_libraries(${EXECUTABLE_NAME} optimized sfml-graphics debug sfml-graphics-d)
	target_link_libraries(${EXECUTABLE_NAME} optimized sfml-window debug sfml-window-d)
	target_link_libraries(${EXECUTABLE_NAME} optimized sfml-system debug sfml-system-d)
	set(GLEW_LIB_DIR ${CMAKE_SOURCE_DIR}/extlibs/GLEW/lib)
	set_property(TARGET GLEW PROPERTY IMPORTED_LOCATION ${GLEW_LIB_DIR}/glew32.lib)
	target_link_libraries(${EXECUTABLE_NAME} GLEW )
	
	include_directories(${CMAKE_SOURCE_DIR}/extlibs/SFML/include)
	include_directories(${CMAKE_SOURCE_DIR}/extlibs/GLEW/include)
else()
	# unsupported compiler
	# TODO: correct this message!
	message("Only Clang, GCC and MSVC compilers are supported at this time. Please set up your compiler settings manually (using cmake) or use one of the supported compilers.")
endif()

find_package (OpenGL 3 REQUIRED)
if (OPENGL_FOUND)
	include_directories(${GL_INCLUDE_DIRS})
	target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
else ()
	message(FATAL_ERROR "Could not find opengl")
endif ()
