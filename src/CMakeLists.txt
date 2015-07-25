
add_executable( ${EXECUTABLE_NAME} 
AttributeCategory.cpp
Camera.cpp
CharacterDrawer.cpp
DefaultFramebuffer.cpp
DemoCore.cpp
DirectionalLight.cpp
FileLoad.cpp
Fog.cpp
Framebuffer.cpp
GraphicalObject.cpp
IFramebuffer.cpp
main.cpp
Mesh.cpp
SimpleColoredDrawer.cpp
Skybox.cpp
Terrain.cpp
TextDrawer.cpp
Water.cpp

AttributeCategory.hpp
Camera.hpp
CharacterDrawer.hpp
DefaultFramebuffer.hpp
DemoCore.hpp
DirectionalLight.hpp
FileLoad.hpp
Fog.hpp
Framebuffer.hpp
GraphicalObject.hpp
IFramebuffer.hpp
Mesh.hpp
SimpleColoredDrawer.hpp
Skybox.hpp
Terrain.hpp
TextDrawer.hpp
Water.hpp)

# Detect and add SFML
# set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
#Find any version 2.X of SFML
#See the FindSFML.cmake file for additional details and instructions
# find_package (SFML 2 OPTIONAL_COMPONENTS system window graphics)
# if (SFML_FOUND)
# 	include_directories(${SFML_INCLUDE_DIR})
# 	target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})
# else ()
# 	message(WARNING "Could not find SFML. Setting link libraries anyway, however this project will not build if SFML libraries can not be found by the compiler.")
# 	set_sfml_library_linker_flags()
# endif ()

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

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	# using Clang or GCC
	set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS "-lsfml-graphics -lsfml-window -lsfml-system -lGLEW -lGL" CACHE STRING "" FORCE)
	
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# using GCC
	message("Project might not build using GCC according to personal experiences. Clang is suggested!")
	set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS "-lsfml-graphics -lsfml-window -lsfml-system -lGLEW -lGL" CACHE STRING "" FORCE)
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
	message("Only clang and MSVC compilers are supported at this time. Please set up your compiler settings manually (using cmake) or use one of the supported compilers.")
endif()

find_package (OpenGL 3 REQUIRED)
if (OPENGL_FOUND)
	include_directories(${GL_INCLUDE_DIRS})
	target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
else ()
	message(FATAL_ERROR "Could not find opengl")
endif ()
