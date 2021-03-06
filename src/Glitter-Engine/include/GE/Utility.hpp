#pragma once

#include <fstream>
#include <string>
#include <vector>

namespace util
{

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
	 * @throws std::runtime_error
	 */
	std::string LoadFileAsString(const std::string& filename);

	/**
	 * \brief Get a list of all files contained in a given directory.
	 *
	 * Iterates through a directory and stores all filenames in a list of std::strings.
	 * Directory does not traversed recursively. Which means that files inside any directory
	 * inside the target directory will not be listed.
	 *
	 * @param dirPath Directory's path where the files should be listed from.
	 *
	 * @return std::vector of filenames (not containing path)
	 *
	 * @throws std::runtime_error
	 */
	std::vector<std::string>  GetFileNamesInDirectory(const std::string& dirPath);

	/**
	 * \brief Class that holds a pointer thats memory is managed by some other facility.
	 *
	 * This class only indicates that lifetime of the object, that it points to, is managed
	 * by some other unit. This class is meant to keep you away from trying to "delete" the
	 * underlying pointer.
	 */
	template<typename T>
	class managed_ptr
	{
	public:
		managed_ptr(T* ptr = nullptr):ptr_(ptr) {}
		T& operator*() {return *ptr_;}
		T* operator->() {return ptr_;}

		T* get_raw() {return ptr_;}

	private:
		T* ptr_;
	};
}
