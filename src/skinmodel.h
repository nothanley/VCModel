#include <fstream>
#include <istream>
#include <vector>
#include <string>
#include "databuffers.h"
#pragma once

struct RigBone;
struct Material;
struct BoundingBox;

class CModelContainer;
class CSkinModel
{
	public:
		CSkinModel();
		CSkinModel(char* data, CModelContainer* pParent);
		~CSkinModel();

	public:
		std::vector<Mesh*>    getMeshes() const { return m_meshes; }
		std::vector<RigBone*> getBones()  const { return m_bones; };
		const std::vector<std::string>* getStringTable() { return &m_stringTable; }
		int  getNumMeshes() { return m_meshes.size(); }
		int  getNumBones()  { return m_bones.size(); }
		int  getNumMats()   { return m_materials.size(); }
		void push_bone(RigBone* bone) { this->m_bones.push_back(bone); }
		void push_mesh(Mesh* mesh)	{ this->m_meshes.push_back(mesh); }

	public:
		void saveToObjFile(const char* path, bool split = true);
		static void saveObjFile(Mesh* mesh, const char* path);

	private:
		void loadData();
		void loadAxisBounds();
		virtual void loadBuffer(){}

	protected:
		CModelContainer* m_parent;
		BoundingBox m_axisBox;

		std::vector<Mesh*> m_meshes;
		std::vector<RigBone*> m_bones;
		std::vector<std::string> m_stringTable;
		std::vector<Material> m_materials;
		
		float m_version;
		int m_type;
		char* m_data;
};



