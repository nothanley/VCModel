#include <common.h>
#include <fstream>
#include <filesystem>
#include <windows.h>
#include "hash.h"

namespace fs = std::filesystem;

char* SysCommon::readBinaryFile(const std::string& filename)
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



bool SysCommon::canWriteToFile(const std::string& filename)
{
	fs::path pathObj(filename);

	// Extract directory path from the given filepath
	std::string directory = pathObj.parent_path().string();
	if (!fs::exists(directory)) {
		if (!fs::create_directories(directory))
			return false;
	}

	// Create the file
	std::ofstream file(filename);
	if (!file.is_open())
		return false;

	return true;
}


std::string SysCommon::getRelativePath(const std::string& path) {
	fs::path currentPath = fs::current_path();
	return currentPath.string() + path;
}


bool SysCommon::containsElement(const std::string& str, const std::vector<std::string>& vec) 
{
	return std::find(vec.begin(), vec.end(), str) != vec.end();
}

uint32_t SysCommon::hash(const std::string str)
{
	size_t size = str.size();
	const char* data = str.c_str();

	uint32_t hash = 5381;

	for (const char* c = data; c < data + size; ++c)
		hash = ((hash << 5) + hash) + (unsigned char)*c;

	return hash;
}

std::string SysCommon::getApplicationPath() {
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::string fullPath(buffer);
	size_t lastSlash = fullPath.find_last_of("\\");
	return fullPath.substr(0, lastSlash);
}


void SysCommon::str_to_lower(std::string& string)
{
	std::transform(string.begin(), string.end(), string.begin(), [](unsigned char c) { return std::tolower(c); });
}

void SysCommon::remove_last_dir(std::string& path)
{
	size_t sPos = path.find_last_of('/');
	if (sPos != std::string::npos)
		path = path.substr(0, sPos);
}

std::string SysCommon::back_slashes_to_forward(const std::string& input)
{
	std::string output;
	for (char ch : input)
	{
		if (ch == '\\')
			output.push_back('/');
		else
			output.push_back(ch);
	}
	return output;
}

uint64_t SysCommon::PathHash(const std::string& path) {
	return Hash::fnv1a64(path.c_str());
}

void SysCommon::rebase_texture_path(std::string& path, const char* target_folder_name)
{
	path = SysCommon::back_slashes_to_forward(path);
	SysCommon::str_to_lower(path);

	size_t pos = path.find("/" + std::string(target_folder_name) + "/");
	if (pos == std::string::npos)
		return;

	std::string rebased_dir = path.substr(pos + 1);
	SysCommon::remove_last_dir(rebased_dir);
	path = rebased_dir + "/textures/";
}

std::string SysCommon::string_prefix(const std::string& str, const char* key)
{
	size_t pos = str.find(key);
	if (pos == std::string::npos)
		return str;

	return str.substr(0, pos);
}

bool SysCommon::fileExists(const std::string& filePath) {
	return std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath);
}

std::string SysCommon::get_parent_folder(const std::string& filePath) 
{
	fs::path path(filePath);
	return path.parent_path().string();
}

std::string SysCommon::replace_prefix(const std::string& texture_map, const std::string& customPrefix) {
	size_t underscorePos = texture_map.find_last_of('_');
	if (underscorePos != std::string::npos) {
		return customPrefix + texture_map.substr(underscorePos + 1);
	}
	return texture_map;
}

std::string SysCommon::replace_substring(const std::string& original, const std::string& toReplace, const std::string& replacement)
{
	std::string result = original;

	// Find the position of the substring to replace
	size_t pos = result.find(toReplace);
	if (pos != std::string::npos) {
		// Replace the substring
		result.replace(pos, toReplace.length(), replacement);
	}

	return result;
}

std::vector<std::string> SysCommon::split(const std::string& input, const std::string& delimiter) {
	std::vector<std::string> result;
	size_t pos = 0;
	size_t start = 0;

	while ((pos = input.find(delimiter, start)) != std::string::npos) {
		result.push_back(input.substr(start, pos - start));
		start = pos + delimiter.length();
	}

	result.push_back(input.substr(start));
	return result;
}