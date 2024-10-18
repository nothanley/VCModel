#include <sstream>
#include <memory>
#include "cmeshserializer.h"
#pragma once

class CModelSerializer : public CMeshSerializer
{
public:
	CModelSerializer(CSkinModel* target);

	void save(const char* path);
	void setUseBlendshapes(const bool use_blendshapes);
	void setNumLods(const int level);

private:
	void formatFile();
	void writeDataBuffer(std::ofstream& fs, const StModelBf& data);

private:
	void generateMeshBuffers(std::vector<StMeshBf>& buffers) override;
	void createTextBuffer();
	void createBoneBuffer();
	void createAtPtBuffer();
	void createMaterialBuffer();
	void createMeshBufferDefs();
	void createLODsBuffer();
	void createModelBuffer();

private:
	void serializePoint(char*& buffer, const StAttachPoint& point);
	void writeBoundingBox(char*& buffer, const BoundingBox& box);
	void writeMeshBuffer(char*& buffer, const StMeshBf& meshBuffer);
	void writeIndexBuffer(char*& buffer, int meshIndex);
	void writeMaterialGroupBuffer(char*& buffer, int meshIndex);

protected:
	void serialize();
	std::string m_savePath;
	int8_t m_numLods;
};



