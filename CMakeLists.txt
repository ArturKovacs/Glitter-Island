cmake_minimum_required (VERSION 2.8)

# FUNCTIONS

function(set_sfml_library_linker_flags)
if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang") OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU"))
	# using Clang or GCC
	set(CMAKE_EXE_LINKER_FLAGS "-lsfml-graphics -lsfml-window -lsfml-system" CACHE STRING "" FORCE)

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	message(WARNING "No support for MSVC at this time")
	
else()
	# unsupported compiler
	# TODO: correct this message!
	message(WARNING "Only clang and MSVC compilers are supported at this time. Please set up your compiler settings manually (using cmake) or use one of the supported compilers.")
endif()
endfunction() 

# ############################
# ############################


project (Glitter-Island)
set(EXECUTABLE_NAME Glitter-Island)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

add_subdirectory(GraphicsProject)

# From cmake 3 ...
# set_property(GLOBAL PROPERTY CXX_STANDARD 11)
# set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

# find_package (OpenGL REQUIRED)
# if (OPENGL_FOUND)
# 	include_directories(${GL_INCLUDE_DIRS})
# 	target_link_libraries (${EXECUTABLE_NAME} ${GL_LIBRARIES})
# else ()
# 	message(FATAL_ERROR "Could not find opengl")
# endif ()

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

if (("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang"))
	# using Clang or GCC
	set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS "-lGL -lGLEW -lsfml-graphics -lsfml-window -lsfml-system" CACHE STRING "" FORCE)
	
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# using GCC
	message("Project might not build using GCC according to personal experiences. Clang is suggested!")
	set(CMAKE_CXX_FLAGS "-std=c++11" CACHE STRING "" FORCE)
	set(CMAKE_EXE_LINKER_FLAGS "-lGL -lGLEW -lsfml-graphics -lsfml-window -lsfml-system" CACHE STRING "" FORCE)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio C++
	message(WARNING "No support for MSVC at this time")
else()
	# unsupported compiler
	# TODO: correct this message!
	message(WARNING "Only clang and MSVC compilers are supported at this time. Please set up your compiler settings manually (using cmake) or use one of the supported compilers.")
endif()

# set(CMAKE_CONFIGURATION_TYPES Release Debug)
# set(DEBUG_CONFIGURATIONS Debug)

# add_custom_target(MyTarget DEPENDS Glitter-Island)

