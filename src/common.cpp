#include "common.h"
#include <fstream>

char*
VCFile::Common::readBinaryFile(const std::string& filename) 
{
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file.is_open())
		return nullptr;

	std::size_t fileSize = static_cast<std::size_t>(file.tellg());
	file.seekg(0, std::ios::beg);

	char* data = new char[fileSize];
	file.read(data, fileSize);
	file.close();
	return data;
}