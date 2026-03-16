#include <vector>
#include <string>
#pragma once 

struct Mesh;
struct RigBone;
struct StMeshBf;
struct BoundingBox;
struct StAttachPoint;

class MeshEncoder
{
protected:
	virtual uint32_t getMDLBufferSize();
	virtual uint32_t getStringBufferSize(const std::vector<std::string>& strings);
	virtual uint32_t getAtPtBufferSize(const std::vector<StAttachPoint>& points);
	virtual uint32_t getMtlBufferSize(const std::vector<Mesh*>& meshes);
	virtual uint32_t getBoneBufferSize(const std::vector<RigBone*>& bones);
	virtual uint32_t getMeshBufferDefSize(std::vector<StMeshBf>& meshbuffers);
	virtual uint32_t getLodsBufferSize(const std::vector<Mesh*>& meshes, int numLevels);
	virtual void updateIndexBufferSize(uint32_t& size, const Mesh* mesh);
};

