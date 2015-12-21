cmake_minimum_required (VERSION 2.8)

if(${CMAKE_VERSION} VERSION_GREATER 3.0)
	cmake_policy(SET CMP0054 OLD)
endif()

project (Glitter-Island)
set(EXECUTABLE_NAME Glitter-Island)
set(ENGINE_NAME Glitter-Engine)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_CONFIGURATION_TYPES Release Debug)
set_property(GLOBAL PROPERTY DEBUG_CONFIGURATIONS Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_HOME_DIRECTORY}/bin-debug)

# copy user file that sets working directory correctly for visual studio projects.
set(SRC_FOLDER_NAME src)
file(READ ${CMAKE_HOME_DIRECTORY}/cmake/additional-files/project.vcxproj.user USER_FILE_CONTENTS)
file(WRITE ${PROJECT_BINARY_DIR}/${SRC_FOLDER_NAME}/${EXECUTABLE_NAME}/${EXECUTABLE_NAME}.vcxproj.user "${USER_FILE_CONTENTS}")

set(USE_PROVIDED_OGLPLUS 1 CACHE BOOL "If true, the compiler will be given the provided, prebuilt oglplus headres. If false, oglplus must be installed on the system.")
set(USE_PROVIDED_GLM 1 CACHE BOOL "If true, the compiler will be given the provided glm headres. If false, glm must be installed on the system.")

# From cmake 3 ...
# set_property(GLOBAL PROPERTY CXX_STANDARD 11)
# set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

add_subdirectory(${SRC_FOLDER_NAME})

# add_custom_target(MyTarget DEPENDS Glitter-Island)

