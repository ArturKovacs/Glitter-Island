cmake_minimum_required (VERSION 2.8)

project (Glitter-Island CXX)

set_property(GLOBAL PROPERTY CXX_STANDARD 11)
set_property(GLOBAL PROPERTY CXX_STANDARD_REQUIRED ON)

# set(CMAKE_CONFIGURATION_TYPES Release Debug)
# set(DEBUG_CONFIGURATIONS Debug)

# add_custom_target(MyTarget DEPENDS Glitter-Island)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_HOME_DIRECTORY}/bin)

add_subdirectory(GraphicsProject)
# add_executable(bin/Glitter-Island GraphicsProject/)

