#include <vector>
#include <sstream>
#include <memory>
#include "databuffers.h"
#pragma once

struct StModelBf {
	std::string type;
	uint32_t size;
	char* data = nullptr;
};

struct StDataBf {
	std::string container;
	std::stringstream stream;

	void setHeader(const std::vector<std::string>& stringTable, 
		const char* data, const char* type, const char* format);

	uint32_t size() { return stream.tellp();}
};

struct StMeshBf {
	Mesh* mesh;
	std::vector< std::shared_ptr<StDataBf> > buffers;
};

class CSkinModel;
class CModelSerializer
{
public:
	CModelSerializer(CSkinModel* target);
	void save(const char* path);

	const CSkinModel* model() { return m_model; }
	const std::vector<std::string>& table() { return m_stringTable; }
	int indexOf(const std::string& target);
	
private:
	void serialize();
	void generateStringTable();

private:
	void createTextBuffer();
	void createBoneBuffer();
	void createMaterialBuffer();
	void createMeshBufferDefs();

protected:
	std::vector<StMeshBf>    m_meshBuffers;
	std::vector<StModelBf>   m_dataBuffers;
	std::vector<std::string> m_stringTable;
	std::string m_savePath;
	CSkinModel* m_model;
};

