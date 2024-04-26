#include <fstream>
#include <istream>
#include "modelcereal.h"
#pragma once

struct RigBone;
struct Material;
struct BoundingBox;

class CModelContainer;
class CSkinModel : public CSerializedModel
{
	public:
		CSkinModel();
		CSkinModel(char* data, CModelContainer* pParent);
		~CSkinModel();

	public:
		std::vector<Mesh*>    getMeshes() const { return m_meshes; }
		std::vector<RigBone*> getBones()  const { return m_bones; };
		const std::vector<std::string>* getStringTable() { return &m_stringTable; }
		const int  getNumMeshes() const { return m_meshes.size(); }
		const int  getNumBones()  const { return m_bones.size(); }
		const int  getNumMats()   const { return m_materials.size(); }
		void push_bone(RigBone* bone) { this->m_bones.push_back(bone); }
		void push_mesh(Mesh* mesh)	{ this->m_meshes.push_back(mesh); }
		const BoundingBox getAABBs();

	public:
		void saveToObjFile(const char* path, bool split = true);
		static void saveObjFile(Mesh* mesh, const char* path);

	protected:
		void loadAxisBounds();
		virtual void loadData();
		virtual void loadBuffer();
		CModelContainer* m_parent;
};



