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
	static int getNumStacks(const StMeshBf& meshBuffer);
	static void writeMatrixToBuffer(char*& buffer, const glm::mat4& matrix);

protected:
	virtual void serialize();
	virtual void formatFile();
	virtual void writeDataBuffer(std::ofstream& fs, const StModelBf& data);
	virtual void createModelBuffer();

protected:
	virtual void generateMeshBuffers(std::vector<StMeshBf>& buffers) override;
	virtual void createTextBuffer();
	virtual void createBoneBuffer();
	virtual void createAtPtBuffer();
	virtual void createMaterialBuffer();
	virtual void createMeshBufferDefs();
	virtual void createLODsBuffer();

protected:
	virtual void writeBoundingBox(char*& buffer, const BoundingBox& box);
	virtual void serializePoint(char*& buffer, const StAttachPoint& point);
	virtual void writeMeshBuffer(char*& buffer, const StMeshBf& meshBuffer);
	virtual void writeIndexBuffer(char*& buffer, int meshIndex);
	virtual void writeMaterialGroupBuffer(char*& buffer, int meshIndex);

protected:
	std::string m_savePath;
	int8_t m_numLods;
};



