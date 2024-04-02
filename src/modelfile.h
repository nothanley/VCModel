/* Validates CAK file and initializes a stream upon validation success  */
#include <fstream>
#pragma once

class CSkinModel;
class CModelContainer {

public:
	CModelContainer(const char* path);
	~CModelContainer();

public:
	CSkinModel* getModel(){ return m_model; }
	uint32_t  getVersion(){ return m_version; }

private:
	void LoadFile();
	void ReadContents();
	void ValidateContainer();

private:
	char* m_fileBf;
	char* m_data;
	bool m_isReady;
	std::string m_sFilePath;
	uint32_t m_version;
	CSkinModel* m_model;
};
