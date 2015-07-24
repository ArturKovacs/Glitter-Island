#include "FileLoad.hpp"

std::string LoadFileAsString(const std::string& filename)
{
	std::ifstream file(filename);

	if (!file.is_open()) {
		throw std::runtime_error((std::string("Can not open file: ") + filename).c_str());
	}

	std::string result;

	file.seekg(0, file.end);
	int size = file.tellg();
	result.resize(size);

	file.seekg(0, file.beg);

	file.read((char*)result.data(), size);

	if (file.fail() && !file.eof()) {
		throw std::runtime_error((std::string("Error occured while reading file: ") + filename).c_str());
	}

	return result;
}
