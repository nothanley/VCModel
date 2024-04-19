#include <sstream>
#include <memory>
#include "cmeshserializer.h"
#pragma once

class CModelSerializer : public CMeshSerializer
{
public:
	CModelSerializer(CSkinModel* target);
	void save(const char* path);

private:
	void generateMeshBuffers(std::vector<StMeshBf>& buffers) override;
	void createTextBuffer();
	void createBoneBuffer();
	void createMaterialBuffer();
	void createMeshBufferDefs();
	
private:
	void writeBoundingBox(char*& buffer, Mesh* mesh);
	void writeMeshBuffer(char*& buffer, const StMeshBf& meshBuffer);

protected:
	void serialize();
	std::string m_savePath;
};

