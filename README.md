
Glitter-Island
==============

Glitter-Island is a personal project that is made for learning and practicing
real-time computer graphics and programming in general. It uses OpenGL for rendering, 
so it supports all three main PC platforms (Linux, Windows, MAC). In the process of 
creating Glitter-Island the primary goal is to make it look amazing, while 
keeping real-time performance on medium range computers. However this goal 
sometimes gets put aside to invest in code quality, portability, and 
occasionally for other regards.

## How to

This section contains some useful information for those who wants to 
try out this little humble program.

### Build

CMake project files are provided. Project files for a desired environment 
can be generated using CMake.

**OpenGL 3.3 is required to build and run this application.**

Theres no dependencies needed to be installed for Visual Studio 2013, project is 
ready to be used out-of-the-box for its compiler.

In case of an other compiler the following dependencies' dev packages are required to be installed:
- SFML of at least version 2.1
- GLEW

A particular configuration of oglplus is provided, but it can be optionally used or 
discarded by setting a cmake variable to the desired value. If the provided oglplus 
is not used then oglplus is required to be installed.

### Use

**OpenGL 3.3 is required to run this application.** If the program does not start, 
it is recommended to update your display driver.

Once the program is running the "standard mode" is active. In this mode the user can 
observe the island. For movement, use [WASD keys](https://en.wikipedia.org/wiki/Arrow_keys#WASD_keys). 
Hold down SHIFT to fly faster. For looking, press space to capture mouse so that mouse movement 
controlls view direction. Press space again to release mouse. 

Table of available keys:

| Key   | Effect                        | Mode   |
|-------|-------------------------------|--------|
| W     | Move forward                  | All    |
| S     | Move backward                 | All    |
| A     | Move left                     | All    |
| D     | Move right                    | All    |
| SHIFT | Move faster                   | All    |
| SPACE | Capture/Release mouse         | All    |
| H     | Toggle wireframe display      | All    |
| E     | Toggle "Editor mode"          | All    |
| 0-9   | Tool selection                | Editor |
