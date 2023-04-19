#include <iostream>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>

#define TYPE double

inline size_t getFileSize(std::string& filepath) {
	std::filesystem::path path(filepath);
	std::filesystem::file_status status = std::filesystem::status(path);
	if (std::filesystem::is_regular_file(status)) {
		return static_cast<size_t>(std::filesystem::file_size(path));
	}
	std::cout << "!!! not a file !!!" << std::endl;
	return 0;
}

void _read(std::ifstream& f, std::string& filepath, const size_t& chuck_size, std::vector<TYPE>* v) {
	std::vector<char> result;
}

inline void read(std::string& filepath, std::vector<TYPE>* v) {
	size_t fileSize = getFileSize(filepath);

	std::ifstream fin(filepath.c_str(), std::ifstream::binary);

	
}

int main() {
	std::string filepath(std::getenv("testcase"));
	std::vector<TYPE> v;
	std::cout << getFileSize(filepath);
	//read(filepath, &v);
	std::cout << "hello";
}
