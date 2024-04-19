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

const std::vector<std::string> STREAM_TABLE {
	"POSITION", "R32_G32_B32","float", "NORMAL", "R8_G8_B8_A8", "snorm",
	"TANGENT","BINORMAL","R8", "COLOR","unorm","TEXCOORD",
	"R32_G32","R16_G16_B16_A16","BLENDINDICES", "uint","BLENDWEIGHTS","R32_G32_B32_A32",""
};

class CSkinModel;
class CModelSerializer
{
public:
	CModelSerializer(CSkinModel* target);
	void save(const char* path);
	
private:
	void serialize();
	void buildStringTable();
	void buildMeshBuffers();

private:
	void createTextBuffer();
	void createBoneBuffer();
	void createMaterialBuffer();
	void createMeshBufferDefs();

private:
	int indexOf(const std::string& target);
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

private:
	inline uint32_t getStringBufferSize(const std::vector<std::string>& strings);
	inline uint32_t getMtlBufferSize(const std::vector<Mesh*>& meshes);
	inline uint32_t getBoneBufferSize(const std::vector<RigBone*>& bones);
	inline uint32_t getMeshBufferDefSize();

protected:
	std::vector<StMeshBf>    m_meshBuffers;
	std::vector<StModelBf>   m_dataBuffers;
	std::vector<std::string> m_stringTable;
	std::string m_savePath;
	CSkinModel* m_model;
};

