#include <vector>
#include <string>

struct Mesh;
struct RigBone;
struct StMeshBf;

class MeshEncoder
{
public:
	static uint32_t getStringBufferSize(const std::vector<std::string>& strings);
	static uint32_t getMtlBufferSize(const std::vector<Mesh*>& meshes);
	static uint32_t getBoneBufferSize(const std::vector<RigBone*>& bones);
	static uint32_t getMeshBufferDefSize(std::vector<StMeshBf>& meshbuffers);
};

