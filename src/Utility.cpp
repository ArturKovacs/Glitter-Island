#include "Utility.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <dirent.h>
#endif

namespace Util
{
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

	std::vector<std::string> GetFileNamesInDirectory(const std::string& dirPath)
	{
		#if defined(_WIN32)

		std::vector<std::string> result;

		std::string findFilePattern = dirPath;

		// Remove all trailing slashes
		while (findFilePattern.back() == '/' || findFilePattern.back() == '\\') {
			findFilePattern = findFilePattern.substr(0, findFilePattern.length()-1);
		}

		// Add slash and wildcard to end
		findFilePattern += "/*";
		//const std::string foundFilePath = findFilePattern.substr(0, findFilePattern.length()-1);

		WIN32_FIND_DATA findFileData;
		HANDLE hFind;

		hFind = FindFirstFile(findFilePattern.c_str(), &findFileData);
		bool findSucceeded = hFind != INVALID_HANDLE_VALUE;

		while (findSucceeded) {
			const bool notDirectory = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0;
			if (notDirectory) {
				result.push_back(findFileData.cFileName);
			}
			findSucceeded = FindNextFile(hFind, &findFileData);
		}

		DWORD error = GetLastError();

		const bool reachedEnd = (error == ERROR_FILE_NOT_FOUND || error == ERROR_NO_MORE_FILES);
		if (reachedEnd) {
			FindClose(hFind);
		}
		else {
			LPSTR msgBuffer = NULL;
			FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPSTR)&msgBuffer, 0, NULL);
			throw std::runtime_error(msgBuffer);
		}

		return result;

		#else

		static_assert(false, "This is an intentionnal compiler error, signaling that this part of the code is still undone.");
		
		//TODO !!!!!!!!
		DIR* dirp = opendir(dirPath.c_str());
		while ((dp = readdir(dirp)) != NULL) {
			if (dp->d_namlen == len && !strcmp(dp->d_name, name)) {
				(void)closedir(dirp);
				return FOUND;
			}
		}
		(void)closedir(dirp);
		return NOT_FOUND;

		#endif
	}
}