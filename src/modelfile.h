/* Validates CAK file and initializes a stream upon validation success  */
#include <fstream>
#pragma once

class CSkinModel;
class CModelContainer {

public:
	CModelContainer(char* data, const size_t size, bool use_lightweight_loader);
	CModelContainer(const char* path);
	~CModelContainer();

public:
	CSkinModel* getModel(){ return m_model; }
	uint32_t  getVersion(){ return m_version; }
	const size_t size() const { return m_fileSize; }
	const char*  end()  const { return m_fileBf + m_fileSize; }
	void free_model();

private:
	void LoadFile();
	void ReadContents();
	void ValidateContainer();

private:
	char* m_fileBf;
	char* m_data;
	bool m_isReady;
	size_t m_fileSize;
	std::string m_sFilePath;
	uint32_t m_version;
	CSkinModel* m_model;
};
