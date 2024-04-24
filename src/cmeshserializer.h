#include "modelcereal.h"
#include <sstream>
#pragma once 

struct StModelBf {

	std::string type;
	uint32_t size;
	char* data = nullptr;

	void free() {
		if (data) 
			delete[] data;
	}

};

struct StDataBf {
	std::string container;
	std::stringstream stream;

	void setHeader(const std::vector<std::string>& stringTable,
		const char* data, const char* type, const char* format);

	uint32_t size() { return stream.tellp(); }
};

struct StMeshBf {
	Mesh* mesh;
	std::vector< std::shared_ptr<StDataBf> > data;
};

class CSkinModel;
class CMeshSerializer
{
protected:
	int indexOf(const std::string& target);
	virtual void generateMeshBuffers(std::vector<StMeshBf>& buffers) = 0;
	void generateStringTable();

protected:
	void serializeVertices(StMeshBf& target);
	void serializeVertexNormals(StMeshBf& target);
	void serializeTangents(StMeshBf& target);
	void serializeBinormals(StMeshBf& target);
	void serializeVertexColors(StMeshBf& target);
	void serializeTexCoords(StMeshBf& target);
	void serializeSkin(StMeshBf& target);
	void serializeVertexRemap(StMeshBf& target);
	void serializeBlendShapes(StMeshBf& target);
	void serializeColorDict(StMeshBf& target);
	void serializeUVDict(StMeshBf& target);

private:
	void writeMeshShapes(std::stringstream& stream, Mesh* mesh);

protected:
	std::vector<StMeshBf>    m_meshBuffers;
	std::vector<StModelBf>   m_dataBuffers;
	std::vector<std::string> m_stringTable;
	CSkinModel* m_model;
};


