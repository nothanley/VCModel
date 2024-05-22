#include "meshstructs.h"
#pragma once

struct StMdlDataRef
{
	Mesh* mesh = nullptr;
	std::string data;   // eg. POSITION, NORMALS, TANGENTS...
	std::string type;   // eg. FLOAT, INT, UINT, etc...
	std::string format; // eg. R8, R32G32, etc..
	uintptr_t address;
};

class CModelContainer;

class CSerializedModel
{
public:
	CSerializedModel(CModelContainer* parent);

public:
	const BoundingBox getAABBs();
	const Vec3 getAttachPointLocalPos(const StAttachPoint& point) const;

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
	static inline void seekToEnd(char*& buffer);

	virtual void loadAttachPtData();
	virtual void loadGtPtData();
	virtual void getMorphWeights(Mesh& mesh);

protected:
	char* m_data;
	float m_version;
	CModelContainer* m_parent;

protected:
	BoundingBox m_axisBox;
	std::vector<std::string> m_stringTable;
	std::vector<Mesh*> m_meshes;
	std::vector<RigBone*> m_bones;
	std::vector<Material> m_materials;
	std::vector<StAttachPoint> m_attachpoints;
	std::vector<StGotoPoint> m_gtpoints;
	std::vector<StMdlDataRef> m_dataRefs;
};


