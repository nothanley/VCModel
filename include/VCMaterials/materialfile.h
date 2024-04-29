#include <fstream>
#include <memory>
#pragma once

class CMaterialLibrary;
class CMaterialFile
{
public:
	CMaterialFile(const char* path);
	~CMaterialFile();

public:
    void load();
	uint32_t getVersion() { return m_version; }
    std::shared_ptr<CMaterialLibrary> getLibrary() { return m_materialLib; }

private:
	void ReadContents();
	void ValidateContainer();

private:
	char* m_fileBf;
	char* m_data;
	bool m_isReady;
	std::string m_sFilePath;
	uint32_t m_version;
    std::shared_ptr<CMaterialLibrary> m_materialLib;
};

