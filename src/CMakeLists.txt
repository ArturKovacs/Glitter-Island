
if (${USE_PROVIDED_OGLPLUS})
	message("Using provided oglplus.")
	include_directories(${CMAKE_SOURCE_DIR}/extlibs/oglplus/include)
endif()

if (${USE_PROVIDED_GLM})
	message("Using provided glm.")
	include_directories(${CMAKE_SOURCE_DIR}/extlibs/glm/include)
endif()

########################################
# Find/set all packages
add_library(sfml-graphics UNKNOWN IMPORTED)
add_library(sfml-window UNKNOWN IMPORTED)
add_library(sfml-system UNKNOWN IMPORTED)
add_library(sfml-graphics-d UNKNOWN IMPORTED)
add_library(sfml-window-d UNKNOWN IMPORTED)
add_library(sfml-system-d UNKNOWN IMPORTED)
add_library(GLEW UNKNOWN IMPORTED)

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
	# using Clang or GCC
	set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "" FORCE)
	
	# Detect SFML
	set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules" ${CMAKE_MODULE_PATH})
	find_package (SFML 2.1 COMPONENTS system window graphics)
	#if (SFML_FOUND)
	#	include_directories(${SFML_INCLUDE_DIR})
	#	target_link_libraries(${EXECUTABLE_NAME} ${SFML_LIBRARIES})
	#else ()
	#	message("Could not find SFML. Setting link libraries anyway, however this project will not build if SFML libraries can not be found by the compiler. (It's recommended to use Clang)")
	#	set(SFML_LIBRARY_LINKER_FLAGS "-lsfml-graphics -lsfml-window -lsfml-system")
	#	set_property(TARGET ${EXECUTABLE_NAME} APPEND_STRING PROPERTY LINK_FLAGS "${SFML_LIBRARY_LINKER_FLAGS}")
	#endif ()
	
	find_package(GLEW)
	#if (GLEW_FOUND)
	#	include_directories(${GLEW_INCLUDE_DIRS})
	#	target_link_libraries(${EXECUTABLE_NAME} ${GLEW_LIBRARIES})
	#else ()
	#	message("Could not find GLEW. Setting link libraries anyway, however this project will not build if GLEW can not be found by the compiler. (It's recommended to use Clang)")
	#	set(GLEW_LIBRARY_LINKER_FLAGS "-lGLEW")
	#	set_property(TARGET ${EXECUTABLE_NAME} APPEND_STRING PROPERTY LINK_FLAGS "${GLEW_LIBRARY_LINKER_FLAGS}")
	#endif ()
	
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	# message("No support for MSVC at this time")
	if((${MSVC_VERSION} LESS 1900))
		message(FATAL_ERROR "Provided libraries are set up for VS 2015. Please use Visual Studio of at least version 14 (2015) or get a previous version of this software from github.")
	endif()
	
	set(SFML_LIB_DIR ${CMAKE_SOURCE_DIR}/extlibs/SFML/lib)
	set_property(TARGET sfml-graphics PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-graphics.lib)
	set_property(TARGET sfml-window PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-window.lib)
	set_property(TARGET sfml-system PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-system.lib)
	set_property(TARGET sfml-graphics-d PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-graphics-d.lib)
	set_property(TARGET sfml-window-d PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-window-d.lib)
	set_property(TARGET sfml-system-d PROPERTY IMPORTED_LOCATION ${SFML_LIB_DIR}/sfml-system-d.lib)
	set(SFML_LIBRARIES 
	optimized sfml-graphics debug sfml-graphics-d
	optimized sfml-window debug sfml-window-d
	optimized sfml-system debug sfml-system-d)
	#target_link_libraries(${EXECUTABLE_NAME} optimized sfml-graphics debug sfml-graphics-d)
	#target_link_libraries(${EXECUTABLE_NAME} optimized sfml-window debug sfml-window-d)
	#target_link_libraries(${EXECUTABLE_NAME} optimized sfml-system debug sfml-system-d)
	
	set(GLEW_LIB_DIR ${CMAKE_SOURCE_DIR}/extlibs/GLEW/lib)
	set_property(TARGET GLEW PROPERTY IMPORTED_LOCATION ${GLEW_LIB_DIR}/glew32.lib)
	set(GLEW_LIBRARIES GLEW)
	#target_link_libraries(${EXECUTABLE_NAME} GLEW )
	
	set(SFML_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/extlibs/SFML/include")
	set(GLEW_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/extlibs/GLEW/include")
	
	#include_directories(${CMAKE_SOURCE_DIR}/extlibs/SFML/include)
	#include_directories(${CMAKE_SOURCE_DIR}/extlibs/GLEW/include)
else()
	# unsupported compiler
	# TODO: correct this message!
	message("Only Clang, GCC and MSVC compilers are supported at this time. Please set up your compiler settings manually (using cmake) or use one of the supported compilers.")
endif()

find_package (OpenGL 3 REQUIRED)
if (NOT OPENGL_FOUND)
	message(FATAL_ERROR "Could not find opengl")
	#include_directories(${GL_INCLUDE_DIRS})
	#target_link_libraries(${EXECUTABLE_NAME} ${OPENGL_LIBRARIES})
endif ()
########################################

add_subdirectory(Glitter-Engine)
add_subdirectory(Glitter-Island)
