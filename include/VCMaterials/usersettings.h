#include <string>
#include "Json.hpp"
#pragma once


class CSettingsJSON
{
public:
	CSettingsJSON(const char* path);

	virtual bool open();
	virtual bool save();

protected:
	template<typename T>
	T getKeyValue(const char* section, const char* name);

	template<typename T>
	void setKeyValue(const char* section, const char* name, T value);

	nlohmann::json& json() { return m_Json; }
	void setupConfig();
	void validateJson();

protected:
	std::string m_filePath;
	nlohmann::json m_Json;
};


