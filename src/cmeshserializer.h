#include <vector>

struct StMeshBf;
class CModelSerializer;

class CMeshSerializer
{
public:
	CMeshSerializer(CModelSerializer* parent);
	void generateMeshBuffers(std::vector<StMeshBf>& buffers);

private:
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
	CModelSerializer* m_parent;
};


