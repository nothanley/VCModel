#include <vector>
#include <string>
#pragma once 

struct Mesh;
struct RigBone;
struct StMeshBf;
struct BoundingBox;

class MeshEncoder
{
public:
	static uint32_t getMDLBufferSize();
	static uint32_t getStringBufferSize(const std::vector<std::string>& strings);
	static uint32_t getMtlBufferSize(const std::vector<Mesh*>& meshes);
	static uint32_t getBoneBufferSize(const std::vector<RigBone*>& bones);
	static uint32_t getMeshBufferDefSize(std::vector<StMeshBf>& meshbuffers);
	static uint32_t getLodsBufferSize(const std::vector<Mesh*>& meshes, int numLevels);

private:
	static void updateIndexBufferSize(uint32_t& size, const Mesh* mesh);
};

