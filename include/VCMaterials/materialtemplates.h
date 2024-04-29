#include "usersettings.h"
#pragma once

class CMaterialLibrary;
struct StMaterial;
struct StTemplate {
	std::string alias;
	std::string shader_name;
	std::string shader_type;
	nlohmann::json object;
};

class CMaterialTemplates : public CSettingsJSON
{
public:
	explicit CMaterialTemplates(const char* path);
	
public:
	bool open() override;
	bool save() override;
	std::vector<std::string> getShaderList();
	std::vector<std::string> getUserPresets(const char* shader_format);
	void mergeMaterials(const std::shared_ptr<CMaterialLibrary>& lib);
	bool addMaterial(const StMaterial& material, const char* alias = "Default");
	StTemplate* getPreset(const char* shader, const char* type, const char* alias = "Default");
	StMaterial getMaterial(const char* shader, const char* type, const char* alias = "Default");

private:
	void loadTemplates();
	void loadTemplateObject(const nlohmann::json& obj);
	void processChildObjects(const nlohmann::json& jsonObject);
	bool hasPreset(const StMaterial& material, const char* alias);
	StTemplate serialize(const StMaterial& material, const char* alias);

private:
	std::vector<StTemplate> m_templates;
};



