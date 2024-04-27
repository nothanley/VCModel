#include <string>
#include <vector>
#pragma once

class SysCommon
{

public:
	static char* readBinaryFile(const std::string& filename);

	static bool canWriteToFile(const std::string& filename);

	static std::string getRelativePath(const std::string& path);

	static bool containsElement(const std::string& str, const std::vector<std::string>& vec);

	static uint32_t hash(const std::string str);

	static std::string getApplicationPath();

	static void str_to_lower(std::string& string);
			    
	static void remove_last_dir(std::string& path);

	static std::string back_slashes_to_forward(const std::string& input);

	static void rebase_texture_path(std::string& path, const char* target_folder_name);

	static uint64_t PathHash(const std::string& path);

	static std::string string_prefix(const std::string& str, const char* key);

	static std::vector<std::string> split(const std::string& input, const std::string& delimiter);

	static bool fileExists(const std::string& filePath);

	static std::string get_parent_folder(const std::string& filePath);

	static std::string replace_prefix(const std::string& texture_map, const std::string& customPrefix);

	static std::string replace_substring(const std::string& original, const std::string& toReplace, const std::string& replacement);
};
