#include <string>
#include <memory>
#pragma once

class CSkinModel;
class CMaterialLibrary;

class CMaterialGen {

public:
	CMaterialGen(CSkinModel* model, const char* templates_path);

public:
	void save(const char* path);

	static std::string get_mtls_path(const std::string model_path);
	static std::string expand_path(const std::string& relativePath);

private:
	void setup_library();

	std::shared_ptr<CMaterialLibrary> m_materials;
	CSkinModel* m_model;
	std::string m_presetsPath;
};