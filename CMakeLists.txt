cmake_minimum_required (VERSION 2.8)
cmake_policy(SET CMP0054 OLD)

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
set(CMAKE_CONFIGURATION_TYPES Release Debug)
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_HOME_DIRECTORY}/Debug)

set(USE_PROVIDED_OGLPLUS 1 CACHE BOOL "If true, the compiler will be given the provided, prebuilt oglplus headres. If false, oglplus must be installed on the system.")

# From cmake 3 ...
# set_property(GLOBAL PROPERTY CXX_STANDARD 11)
# set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)

# add_custom_target(MyTarget DEPENDS Glitter-Island)

