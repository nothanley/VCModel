/* Validates CAK file and initializes a stream upon validation success  */
#include <fstream>
#pragma once

namespace enModelDefs 
{
	enum Enum {
		LoadNormal,
		LoadLightWeight
	};
};

class CSkinModel;
class CModelContainer 
{
public: 
	CModelContainer(char* data, const size_t size, bool use_lightweight_loader);
	CModelContainer(const char* path, bool use_lightweight_loader=false);
	~CModelContainer();

public:
	void load();
	void refresh(); // Use 'refresh()' after injecting vertex/normal data to cskinmodel
	std::shared_ptr<CSkinModel> getModel(){ return m_model; }
	uint32_t getVersion(){ return m_version; }
	const size_t size() const { return m_fileSize; }
	const char* data()  const { return m_fileBf; }
	const char*  end()  const { return m_fileBf + m_fileSize; }
	std::string path() const { return m_sFilePath; }
	void free_model();
	int getLoadType();
	 
private:
	void readModel();
	void readYukes();
	void validateFile();

private:
	char* m_fileBf;
	char* m_data;
	int m_loadType;
	bool m_isReady;
	size_t m_fileSize;
	std::string m_sFilePath;
	uint32_t m_version;
	uint32_t m_signature;
	std::shared_ptr<CSkinModel> m_model;
};
