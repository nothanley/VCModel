#include "meshstructs.h"
#pragma once

class CSerializedModel
{
public:
	CSerializedModel();

protected:
	void loadMeshes();
	void loadStringTable();
	void loadMaterials();
	void loadLods();

	virtual void buildMesh(Mesh& mesh);
	virtual void getMeshMapInfo(Mesh& mesh) {};
	virtual void loadModelBones(const uintptr_t& size) {};
	virtual void readBone() {};

protected:
	RigBone* loadBoneTransform(char*& buffer);
	void getAxisAlignedBoundingBox(Mesh& mesh, bool getRadius = true);
	void loadMeshDef(char* data, const MeshDefBf& def, Mesh& mesh);
	void loadMeshData(Mesh& mesh);
	void getSkinData(Mesh& mesh);
	void getVertexRemap(Mesh& mesh);
	void getTriangleBuffer(Mesh& mesh);
	void loadColorMapInfo(Mesh& mesh);
	void loadUVInfo(Mesh& mesh);
	virtual void getMorphWeights(Mesh& mesh);

protected:
	char* m_data;
	float m_version;
	BoundingBox m_axisBox;
	std::vector<std::string> m_stringTable;
	std::vector<Mesh*> m_meshes;
	std::vector<RigBone*> m_bones;
	std::vector<Material> m_materials;
};


