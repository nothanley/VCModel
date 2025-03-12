#include "modelcereal.h"
#include "meshencoder.h"
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
class CMeshSerializer : public MeshEncoder
{
protected:
	int indexOf(const std::string& target);
	virtual void generateMeshBuffers(std::vector<StMeshBf>& buffers) = 0;
	void generateStringTable();

protected:
	virtual void serializeVertices(StMeshBf& target);
	virtual void serializeVertexNormals(StMeshBf& target);
	virtual void serializeTangents(StMeshBf& target);
	virtual void serializeBinormals(StMeshBf& target);
	virtual void serializePackedTbn(StMeshBf& target);
	virtual void serializeVertexColors(StMeshBf& target);
	virtual void serializeTexCoords(StMeshBf& target);
	virtual void serializeSkin(StMeshBf& target);
	virtual void serializeVertexRemap(StMeshBf& target);
	virtual void serializeBlendShapes(StMeshBf& target);
	virtual void serializeColorDict(StMeshBf& target);
	virtual void serializeUVDict(StMeshBf& target);

private:
	void writeMeshShapes(std::stringstream& stream, Mesh* mesh);

protected:
	bool m_exportBlendshapes;
	bool m_exportCalcTangents;
	std::vector<StMeshBf>    m_meshBuffers;
	std::vector<StModelBf>   m_dataBuffers;
	std::vector<std::string> m_stringTable;
	CSkinModel* m_model;

};


