#pragma once

#include <fstream>
#include <string>

/**
 * \brief Loads a file's contntets to an std::string.
 *
 * This function takes a filename that optionally contains an arbitrary path.
 * If the file loading is not succesful, an exception is thorwn containing a description of the cause.
 * 
 * @param filename File's name that is needed to be read.
 *
 * @return File's contents as a string.
 *
 * @throws std::exception
 */
std::string LoadFileAsString(const std::string& filename);
